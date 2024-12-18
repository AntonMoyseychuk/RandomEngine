#include "pch.h"
#include "shader_mng.h"

#include "utils/data_structures/hash.h"

#include "utils/debug/assertion.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"


#if defined(ENG_DEBUG)
    #define CHECK_UNIFORM(pUniform, searchName)                                                         \
        if (!pUniform) {                                                                                \
            ENG_LOG_GRAPHICS_API_ERROR("Shader program doesn't have '{}' uniform", searchName.CStr());  \
            return;                                                                                     \
        }
#else
    #define CHECK_UNIFORM(pUniform)
#endif


static constexpr size_t ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT = 4096; // TODO: make it configurable
static constexpr size_t ENG_MAX_UNIFORM_NAME_LENGTH = 128;             // TODO: make it configurable


static std::unique_ptr<ShaderManager> g_pShaderMng = nullptr;


static GLenum ToOpenGLNativeShaderStageType(ShaderStageType type) noexcept
{
    switch (type) {
    case ShaderStageType::VERTEX: return GL_VERTEX_SHADER;
    case ShaderStageType::PIXEL:  return GL_FRAGMENT_SHADER;
    
    default:
        ENG_ASSERT_GRAPHICS_API_FAIL("Invalid ShaderStageType value: {}", static_cast<uint32_t>(type));
        return GL_NONE;
    }
}


class ShaderStage
{
    friend class ShaderProgram;

public:
    ShaderStage(const ShaderStage& other) = delete;
    ShaderStage& operator=(const ShaderStage& other) = delete;

    ShaderStage() = default;
    ~ShaderStage() { Destroy(); }

    ShaderStage(ShaderStage&& other) noexcept;
    ShaderStage& operator=(ShaderStage&& other) noexcept;

    bool Init(const ShaderStageCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

    bool IsValid() const noexcept { return m_id != 0; }

private:
    std::string PreprocessSourceCode(const ShaderStageCreateInfo& createInfo) const noexcept;

    bool GetCompilationStatus() const noexcept;

private:
    uint32_t m_id = 0;
};


ShaderStage::ShaderStage(ShaderStage &&other) noexcept
{
    std::swap(m_id, other.m_id);
}


ShaderStage &ShaderStage::operator=(ShaderStage &&other) noexcept
{
    Destroy();
    std::swap(m_id, other.m_id);
    
    return *this;
}


bool ShaderStage::Init(const ShaderStageCreateInfo &createInfo) noexcept
{
    const GLenum shaderStageGLType = ToOpenGLNativeShaderStageType(createInfo.type);
    
    if (shaderStageGLType == GL_NONE) {
        return false;
    }

    const std::string preprocessedSourceCode = PreprocessSourceCode(createInfo);
    if (preprocessedSourceCode.empty()) {
        ENG_LOG_GRAPHICS_API_WARN("Empty shader source code");
        return false;
    }

    if (IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Calling ShaderStage::Create(const ShaderStageCreateInfo&) by shader stage with valid id: {}", m_id);
        Destroy();
    }

    m_id = glCreateShader(shaderStageGLType);

    const char* pPreprocSourceCode = preprocessedSourceCode.c_str();
    const int32_t preprocSourceCodeSize = preprocessedSourceCode.size();

    glShaderSource(m_id, 1, &pPreprocSourceCode, &preprocSourceCodeSize);
    glCompileShader(m_id);

    const bool compilationSuccess = GetCompilationStatus();
    if (!compilationSuccess) {
        Destroy();
    }

    return compilationSuccess;
}


void ShaderStage::Destroy() noexcept
{
    glDeleteShader(m_id);
    m_id = 0;
}


std::string ShaderStage::PreprocessSourceCode(const ShaderStageCreateInfo& createInfo) const noexcept
{
    ENG_ASSERT_GRAPHICS_API(createInfo.pSourceCode, "Source code is nullptr");

    if (createInfo.definesCount == 0) {
        return createInfo.pSourceCode;
    }

    std::stringstream ss;

    static std::regex versionRegex("#version\\s*(\\d+)");

    const std::string_view sourceCodeStrView = createInfo.pSourceCode;

    std::match_results<std::string_view::const_iterator> match;
    const bool matchFound = std::regex_search(sourceCodeStrView.begin(), sourceCodeStrView.end(), match, versionRegex);

    ENG_ASSERT_GRAPHICS_API(matchFound, "Shader error: #version is missed");

    const int32_t version = std::stoi(match[1]);

    const size_t versionPos = sourceCodeStrView.find("#version");
    const std::string_view afterVersion = sourceCodeStrView.substr(versionPos + match.length());

    ss << "#version " << version << '\n';

    for (size_t i = 0; i < createInfo.definesCount; ++i) {
        const char* pDefineStr = createInfo.pDefines[i];
        ENG_ASSERT_GRAPHICS_API(pDefineStr, "pDefineStr string is nullptr");
            
        ss << "#define " << pDefineStr << '\n';
    }

    ss << afterVersion;

    return ss.str();
}


bool ShaderStage::GetCompilationStatus() const noexcept
{
    if (!IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Checking compile status by shader stage with invalid id: {}", m_id);
        return false;
    }

    GLint successStatus;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &successStatus);

    if (!successStatus) {
#if defined(ENG_LOGGING_ENABLED) 
        GLchar infoLog[512] = { 0 };
        glGetShaderInfoLog(m_id, sizeof(infoLog), nullptr, infoLog);

        ENG_LOG_GRAPHICS_API_ERROR("Shader stage (id: {}) compilation error: {}", m_id, infoLog);
#endif

        return false;
    }

    return true;
}


bool ProgramUniformStorage::HasUniform(ds::StrID uniformName) const noexcept
{
    for (const ProgramUniform& uniform : m_uniforms) {
        if (uniform.m_name == uniformName) {
            return true;
        }
    }

    return false;
}


const ProgramUniform* ProgramUniformStorage::GetUniform(ds::StrID uniformName) const noexcept
{
    for (const ProgramUniform& uniform : m_uniforms) {
        if (uniform.m_name == uniformName) {
            return &uniform;
        }
    }

    return nullptr;
}


void ProgramUniformStorage::SetUniformBool(ds::StrID uniformName, bool value) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform1i(m_programRenderID, pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformInt(ds::StrID uniformName, int32_t value) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform1i(m_programRenderID, pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformInt2(ds::StrID uniformName, int32_t x, int32_t y) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform2i(m_programRenderID, pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformInt3(ds::StrID uniformName, int32_t x, int32_t y, int32_t z) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform3i(m_programRenderID, pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformInt4(ds::StrID uniformName, int32_t x, int32_t y, int32_t z, int32_t w) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform4i(m_programRenderID, pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformUInt(ds::StrID uniformName, uint32_t value) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform1ui(m_programRenderID, pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformUInt2(ds::StrID uniformName, uint32_t x, uint32_t y) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform2ui(m_programRenderID, pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformUInt3(ds::StrID uniformName, uint32_t x, uint32_t y, uint32_t z) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform3ui(m_programRenderID, pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformUInt4(ds::StrID uniformName, uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform4ui(m_programRenderID, pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformFloat(ds::StrID uniformName, float value) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform1f(m_programRenderID, pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformFloat2(ds::StrID uniformName, float x, float y) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform2f(m_programRenderID, pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformFloat3(ds::StrID uniformName, float x, float y, float z) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform3f(m_programRenderID, pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformFloat4(ds::StrID uniformName, float x, float y, float z, float w) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform4f(m_programRenderID, pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformDouble(ds::StrID uniformName, double value) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform1d(m_programRenderID, pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformDouble2(ds::StrID uniformName, double x, double y) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform2d(m_programRenderID, pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformDouble3(ds::StrID uniformName, double x, double y, double z) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform3d(m_programRenderID, pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformDouble4(ds::StrID uniformName, double x, double y, double z, double w) noexcept
{
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName);

    glProgramUniform4d(m_programRenderID, pUniform->m_location, x, y, z, w);
}


ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
    std::swap(m_renderID, other.m_renderID);
}


ShaderProgram &ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    Destroy();
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void ShaderProgram::Bind() const noexcept
{
    // TODO: To think about global OpenGL state to check if it's already what we need and reduce driver calls
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Trying to bind invalid shader program");
    glUseProgram(m_renderID);
}


void ShaderProgram::Unbind() const noexcept
{
    glUseProgram(0);
}


ProgramUniformStorage &ShaderProgram::GetUniformStorage() noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Trying to get uniform storage from invalid shader program");
    return *m_pUniformStorage;
}


uint64_t ShaderProgram::Hash() const noexcept
{
    return amHash(m_renderID);
}


bool ShaderProgram::Init(const ShaderProgramCreateInfo &createInfo) noexcept
{
    if (!createInfo.pStageCreateInfos || createInfo.stageCreateInfosCount == 0) {
        ENG_ASSERT_GRAPHICS_API_FAIL("pStages is nullptr or zero sized");
        return false;
    }

    if (IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Calling ShaderProgram::Create(const ShaderProgramCreateInfo&) by shader program with valid id: {}", m_renderID);
        Destroy();
    }

    std::array<ShaderStage, static_cast<size_t>(ShaderStageType::COUNT)> shaderStages = {};
    for (size_t i = 0; i < createInfo.stageCreateInfosCount; ++i) {
        const ShaderStageCreateInfo* pStageCreateInfo = createInfo.pStageCreateInfos[i];
        
        ENG_ASSERT_GRAPHICS_API(pStageCreateInfo, "pStageCreateInfo is nullptr");
        
        if (!shaderStages[i].Init(*pStageCreateInfo)) {
            return false;
        }
    }

    m_renderID = glCreateProgram();
    
    for (size_t i = 0; i < createInfo.stageCreateInfosCount; ++i) {
        glAttachShader(m_renderID, shaderStages[i].m_id);
    }

    glLinkProgram(m_renderID);

    const bool linkingSuccess = GetLinkingStatus();
    if (!linkingSuccess) {
        Destroy();
    }

    return linkingSuccess;
}


void ShaderProgram::Destroy() noexcept
{
    glDeleteProgram(m_renderID);
    m_renderID = 0;
}


bool ShaderProgram::GetLinkingStatus() const noexcept
{
    if (!IsValidRenderID()) {
        ENG_LOG_GRAPHICS_API_ERROR("Invalid shader program id");
        return false;
    }

    glValidateProgram(m_renderID);

    GLint linkSuccess;
    glGetProgramiv(m_renderID, GL_LINK_STATUS, &linkSuccess);

    if (!linkSuccess) {
#if defined(ENG_LOGGING_ENABLED)
        GLchar infoLog[512] = { 0 };
        glGetProgramInfoLog(m_renderID, sizeof(infoLog), nullptr, infoLog);

        ENG_LOG_GRAPHICS_API_ERROR("Shader program (id: {}) linking error: {}", m_renderID, infoLog);
#endif

        return false;
    }

    return true;
}


ShaderManager &ShaderManager::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsShaderManagerInitialized(), "Shader manager is not initialized");
    return *g_pShaderMng;
}


ShaderManager::~ShaderManager()
{
    Terminate();
}


ProgramID ShaderManager::RegisterShaderProgram(const ShaderProgramCreateInfo &createInfo) noexcept
{
    ProgramID id(amHash(createInfo));

    auto& programStorage = m_shaderProgramsStorage;  

    if (programStorage.find(id) != programStorage.cend()) {
        return id;
    }

    ENG_ASSERT_GRAPHICS_API(float(programStorage.size() + 1ull) / programStorage.bucket_count() < programStorage.max_load_factor(), "Shader program storage rehashing: ShaderProgram pointers will be invalidated");

    ShaderProgram& program = programStorage[id];

    if (!program.Init(createInfo)) {
        programStorage.erase(id);
        return ProgramID{};
    }

    ProgramUniformStorage& uniformStorage = m_uniformsStorage[id];
    FillProgramUniformStorage(program, uniformStorage);

    program.m_pUniformStorage = &uniformStorage;

    return id;
}


void ShaderManager::UnregisterShaderProgram(const ProgramID &id) noexcept
{
    if (m_shaderProgramsStorage.find(id) == m_shaderProgramsStorage.cend()) {
        return;
    }

    m_shaderProgramsStorage.erase(id);
}


ShaderProgram* ShaderManager::GetShaderProgramByID(const ProgramID &id) noexcept
{
    return m_shaderProgramsStorage.find(id) != m_shaderProgramsStorage.end() ? &m_shaderProgramsStorage[id] : nullptr;
}


const ProgramUniformStorage* ShaderManager::GetShaderProgramUniformStorageByID(const ProgramID &id) const noexcept
{
    const auto& storageIt = m_uniformsStorage.find(id);
    return storageIt != m_uniformsStorage.cend() ? &storageIt->second : nullptr;
}


bool ShaderManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_shaderProgramsStorage.reserve(ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT);
    m_uniformsStorage.reserve(ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT);
    m_isInitialized = true;

    return true;
}


void ShaderManager::Terminate() noexcept
{
    m_shaderProgramsStorage.clear();
    m_isInitialized = false;
}


void ShaderManager::FillProgramUniformStorage(const ShaderProgram &program, ProgramUniformStorage &storage) noexcept
{
    storage.m_programRenderID = program.m_renderID;

    GLint activeUniformsCount = 0;
    glGetProgramiv(program.m_renderID, GL_ACTIVE_UNIFORMS, &activeUniformsCount);

    if (activeUniformsCount == 0) {
        return;
    }

    std::vector<ProgramUniform>& uniforms = storage.m_uniforms;
    uniforms.reserve(activeUniformsCount);
    uniforms.clear();

    GLsizei nameLength = 0;
	GLsizei count = 0;
	GLenum 	type = GL_NONE;

    for (GLint i = 0; i < activeUniformsCount; ++i) {
        GLchar pUniformName[ENG_MAX_UNIFORM_NAME_LENGTH] = {};

		glGetActiveUniform(program.m_renderID, i, sizeof(pUniformName), &nameLength, &count, &type, pUniformName);

		ProgramUniform uniform = {};
        uniform.m_name = pUniformName;
		uniform.m_location = glGetUniformLocation(program.m_renderID, pUniformName);

    #if defined(ENG_DEBUG)
		uniform.m_count = count;
        uniform.m_type = [type]() -> ProgramUniform::Type {
            switch(type) {
                case GL_FLOAT:              return ProgramUniform::TYPE_FLOAT;
                case GL_DOUBLE:             return ProgramUniform::TYPE_DOUBLE;
                case GL_INT:                return ProgramUniform::TYPE_INT;
                case GL_UNSIGNED_INT:       return ProgramUniform::TYPE_UINT;
                case GL_BOOL:               return ProgramUniform::TYPE_BOOL;
                case GL_FLOAT_VEC2:         return ProgramUniform::TYPE_FVEC2;
                case GL_FLOAT_VEC3:         return ProgramUniform::TYPE_FVEC3;
                case GL_FLOAT_VEC4:         return ProgramUniform::TYPE_FVEC4;
                case GL_DOUBLE_VEC2:        return ProgramUniform::TYPE_DVEC2;
                case GL_DOUBLE_VEC3:        return ProgramUniform::TYPE_DVEC3;
                case GL_DOUBLE_VEC4:        return ProgramUniform::TYPE_DVEC4;
                case GL_INT_VEC2:           return ProgramUniform::TYPE_IVEC2;
                case GL_INT_VEC3:           return ProgramUniform::TYPE_IVEC3;
                case GL_INT_VEC4:           return ProgramUniform::TYPE_IVEC4;
                case GL_UNSIGNED_INT_VEC2:  return ProgramUniform::TYPE_UVEC2;
                case GL_UNSIGNED_INT_VEC3:  return ProgramUniform::TYPE_UVEC3;
                case GL_UNSIGNED_INT_VEC4:  return ProgramUniform::TYPE_UVEC4;
                case GL_FLOAT_MAT2:         return ProgramUniform::TYPE_MAT2X2;
                case GL_FLOAT_MAT3:         return ProgramUniform::TYPE_MAT3X3;
                case GL_FLOAT_MAT4:         return ProgramUniform::TYPE_MAT4X4;
                case GL_FLOAT_MAT2x3:       return ProgramUniform::TYPE_MAT2X3;
                case GL_FLOAT_MAT2x4:       return ProgramUniform::TYPE_MAT2X4;
                case GL_FLOAT_MAT3x2:       return ProgramUniform::TYPE_MAT3X2;
                case GL_FLOAT_MAT3x4:       return ProgramUniform::TYPE_MAT3X4;
                case GL_FLOAT_MAT4x2:       return ProgramUniform::TYPE_MAT4X2;
                case GL_FLOAT_MAT4x3:       return ProgramUniform::TYPE_MAT4X3;
                case GL_DOUBLE_MAT2:        return ProgramUniform::TYPE_DMAT2X2;
                case GL_DOUBLE_MAT3:        return ProgramUniform::TYPE_DMAT3X3;
                case GL_DOUBLE_MAT4:        return ProgramUniform::TYPE_DMAT4X4;
                case GL_DOUBLE_MAT2x3:      return ProgramUniform::TYPE_DMAT2X3;
                case GL_DOUBLE_MAT2x4:      return ProgramUniform::TYPE_DMAT2X4;
                case GL_DOUBLE_MAT3x2:      return ProgramUniform::TYPE_DMAT3X2;
                case GL_DOUBLE_MAT3x4:      return ProgramUniform::TYPE_DMAT3X4;
                case GL_DOUBLE_MAT4x2:      return ProgramUniform::TYPE_DMAT4X2;
                case GL_DOUBLE_MAT4x3:      return ProgramUniform::TYPE_DMAT4X3;
                case GL_SAMPLER_1D:         return ProgramUniform::TYPE_SAMPLER_1D;
                case GL_SAMPLER_2D:         return ProgramUniform::TYPE_SAMPLER_2D;
                case GL_SAMPLER_3D:         return ProgramUniform::TYPE_SAMPLER_3D;
                case GL_SAMPLER_CUBE:       return ProgramUniform::TYPE_SAMPLER_CUBE;
                case GL_IMAGE_1D:           return ProgramUniform::TYPE_IMAGE_1D;
                case GL_IMAGE_2D:           return ProgramUniform::TYPE_IMAGE_2D;
                case GL_IMAGE_3D:           return ProgramUniform::TYPE_IMAGE_3D;
                case GL_IMAGE_2D_RECT:      return ProgramUniform::TYPE_IMAGE_2D_RECT;
                case GL_IMAGE_CUBE:         return ProgramUniform::TYPE_IMAGE_CUBE;
                case GL_IMAGE_BUFFER:       return ProgramUniform::TYPE_IMAGE_BUFFER;
                case GL_IMAGE_1D_ARRAY:     return ProgramUniform::TYPE_IMAGE_1D_ARRAY;
                case GL_IMAGE_2D_ARRAY:     return ProgramUniform::TYPE_IMAGE_2D_ARRAY;
                case GL_IMAGE_2D_MULTISAMPLE:       return ProgramUniform::TYPE_IMAGE_2D_MULTISAMPLE;
                case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return ProgramUniform::TYPE_IMAGE_2D_MULTISAMPLE_ARRAY;
                default: 
                    ENG_ASSERT_GRAPHICS_API_FAIL("Invalid OpenGL uniform type: {}", static_cast<uint32_t>(type)); 
                    return ProgramUniform::TYPE_INVALID;
            };
        }();
    #endif

		uniforms.emplace_back(uniform);
	}
}


bool ShaderManager::IsInitialized() const noexcept
{
    return m_isInitialized;
}


uint64_t amHash(const ShaderStageCreateInfo &stageCreateInfo) noexcept
{
    ds::HashBuilder builder;

    builder.AddValue(stageCreateInfo.type);
    builder.AddMemory(stageCreateInfo.pSourceCode, stageCreateInfo.codeSize);

    for (size_t i = 0; i < stageCreateInfo.definesCount; ++i) {
        const char* pDefine = stageCreateInfo.pDefines[i];

        builder.AddMemory(pDefine, strlen(pDefine));
    }

    return builder.Value();
}


uint64_t amHash(const ShaderProgramCreateInfo &programCreateInfo) noexcept
{
    ds::HashBuilder builder;

    for (size_t i = 0; i < programCreateInfo.stageCreateInfosCount; ++i) {
        const ShaderStageCreateInfo& stageCreateInfo = *programCreateInfo.pStageCreateInfos[i];
        builder.AddValue(stageCreateInfo);
    }

    return builder.Value();
}


bool engInitShaderManager() noexcept
{
    if (engIsShaderManagerInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("Shader manager is already initialized!");
        return true;
    }

    g_pShaderMng = std::unique_ptr<ShaderManager>(new ShaderManager);

    if (!g_pShaderMng) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to create shader manager");
        return false;
    }

    if (!g_pShaderMng->Init()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialized shader manager");
        return false;
    }

    return true;
}


void engTerminateShaderManager() noexcept
{
    g_pShaderMng = nullptr;
}


bool engIsShaderManagerInitialized() noexcept
{
    return g_pShaderMng && g_pShaderMng->IsInitialized();
}

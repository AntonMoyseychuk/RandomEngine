#include "pch.h"
#include "shader_mng.h"

#include "utils/data_structures/hash.h"
#include "utils/file/file.h"

#include "utils/debug/assertion.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"


#if defined(ENG_DEBUG)
    static bool CheckUniform(const ProgramUniform* pUniform, ds::StrID name, ProgramUniform::Type type, uint32_t count) noexcept
    {
        if (!pUniform) {
            ENG_LOG_GRAPHICS_API_ERROR("Shader program doesn't have '{}' uniform", name.CStr());
            return false;
        }

        if (pUniform->GetType() != type) {                                                          
            ENG_LOG_GRAPHICS_API_ERROR("Uniform '{}' type mismatch", name.CStr());              
            return false;                                                                                   
        }

        if (pUniform->GetCount() < count) {
            ENG_LOG_GRAPHICS_API_ERROR("Uniform '{}' count mismatch", name.CStr());
            return false;
        }

        return true;
    }

    #define CHECK_UNIFORM(pUniform, name, type, count) if (!CheckUniform(pUniform, name, type, count)) { return; }
#else
    #define CHECK_UNIFORM(pUniform, name, type, count)
#endif


static constexpr size_t ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT = 4096; // TODO: make it configurable
static constexpr size_t ENG_MAX_UNIFORM_NAME_LENGTH = 128;             // TODO: make it configurable
static constexpr size_t ENG_MAX_SHADER_INCLUDE_DEPTH = 128;            // TODO: make it configurable


static std::unique_ptr<ShaderManager> g_pShaderMng = nullptr;


static void Preprocessor_GetShaderVersionPosition(const std::string_view& sourceCode, ptrdiff_t& begin, ptrdiff_t& end) noexcept
{
    ENG_ASSERT_GRAPHICS_API(!sourceCode.empty(), "Source code is empty string view");

    static std::regex versionRegex("#version\\s*(\\d+)");

    std::match_results<std::string_view::const_iterator> versionMatch;
    const bool versionFound = std::regex_search(sourceCode.begin(), sourceCode.end(), versionMatch, versionRegex);

    ENG_ASSERT_GRAPHICS_API(versionFound, "Shader preprocessing error: #version is missed");
    
    begin = versionMatch.prefix().length();
    end = begin + versionMatch.length() + 1;
}


static void Preprocessor_FillIncludes(std::stringstream& ss, const std::string_view& sourceCode, const fs::path& includeDirPath, size_t includeDepth = 0) noexcept
{
    ENG_ASSERT_GRAPHICS_API(includeDepth < ENG_MAX_SHADER_INCLUDE_DEPTH, "Shader include recursion depth overflow");

    static std::regex includeRegex("#include\\s*[\"<](.*?)[\">]");

    std::cregex_iterator includeIter = std::cregex_iterator(
        sourceCode.data(), sourceCode.data() + sourceCode.size(), includeRegex);

    std::string_view curentSourceCode = sourceCode;

    ptrdiff_t currentIncludePos;
    ptrdiff_t prevIncludePos = 0;

    std::vector<char> includeFileContent;

    for (; includeIter != std::cregex_iterator(); ++includeIter) {
        const std::cmatch& mr = *includeIter;
            
        currentIncludePos = mr.position(0);

        if (currentIncludePos != prevIncludePos) {
            ss.write(sourceCode.data() + prevIncludePos, currentIncludePos - prevIncludePos) << '\n';
        }

        prevIncludePos = currentIncludePos + mr.length(0);
        
        curentSourceCode = sourceCode.substr(mr.position() + mr.length());

        std::string_view includeFile(mr[1].first, mr[1].length());
        const fs::path includeFilepath = includeDirPath / includeFile;

        ReadTextFile(includeFilepath, includeFileContent);

        if (!includeFileContent.empty()) {
            Preprocessor_FillIncludes(ss, includeFileContent.data(), includeDirPath, includeDepth + 1);
        }
    }

    ss << curentSourceCode << '\n';
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

    bool IsValid() const noexcept { return m_stageID != 0; }

private:
    static std::string PreprocessSourceCode(const ShaderStageCreateInfo& createInfo) noexcept;

private:
    bool GetCompilationStatus() const noexcept;

private:
    uint32_t m_stageID = 0;
};


ShaderStage::ShaderStage(ShaderStage &&other) noexcept
{
    std::swap(m_stageID, other.m_stageID);
}


ShaderStage &ShaderStage::operator=(ShaderStage &&other) noexcept
{
    Destroy();
    std::swap(m_stageID, other.m_stageID);
    
    return *this;
}


bool ShaderStage::Init(const ShaderStageCreateInfo &createInfo) noexcept
{
    const GLenum shaderStageGLType = [](ShaderStageType type) -> GLenum {
        switch (type) {
            case ShaderStageType::VERTEX: return GL_VERTEX_SHADER;
            case ShaderStageType::PIXEL:  return GL_FRAGMENT_SHADER;
            default:
                ENG_ASSERT_GRAPHICS_API_FAIL("Invalid ShaderStageType value: {}", static_cast<uint32_t>(type));
                return GL_NONE;
        }
    }(createInfo.type);
    
    if (shaderStageGLType == GL_NONE) {
        return false;
    }

    const std::string preprocessedSourceCode = PreprocessSourceCode(createInfo);
    if (preprocessedSourceCode.empty()) {
        ENG_LOG_GRAPHICS_API_WARN("Empty shader source code");
        return false;
    }

    if (IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Recreation of shader stage: {}", m_stageID);
        Destroy();
    }

    m_stageID = glCreateShader(shaderStageGLType);

    const char* pPreprocSourceCode = preprocessedSourceCode.c_str();
    const int32_t preprocSourceCodeSize = preprocessedSourceCode.size();

    glShaderSource(m_stageID, 1, &pPreprocSourceCode, &preprocSourceCodeSize);
    glCompileShader(m_stageID);

    const bool compilationSuccess = GetCompilationStatus();
    if (!compilationSuccess) {
        Destroy();
    }

    return compilationSuccess;
}


void ShaderStage::Destroy() noexcept
{
    glDeleteShader(m_stageID);
    m_stageID = 0;
}


std::string ShaderStage::PreprocessSourceCode(const ShaderStageCreateInfo& createInfo) noexcept
{
    ENG_ASSERT_GRAPHICS_API(createInfo.pSourceCode, "Source code is nullptr");

    std::stringstream ss;

    std::string_view sourceCode = createInfo.pSourceCode;
    
    ptrdiff_t versionPatternBeginPos, versionPatternEndPos;
    Preprocessor_GetShaderVersionPosition(sourceCode, versionPatternBeginPos, versionPatternEndPos);
    const std::streamsize versionPatternSize = versionPatternEndPos - versionPatternBeginPos;

    ss.write(sourceCode.data() + versionPatternBeginPos, versionPatternSize);

    sourceCode = sourceCode.data() + versionPatternBeginPos + versionPatternSize;

    for (size_t i = 0; i < createInfo.definesCount; ++i) {
        const char* pDefineStr = createInfo.pDefines[i];
        ENG_ASSERT_GRAPHICS_API(pDefineStr, "pDefineStr string is nullptr");
            
        ss << "#define " << pDefineStr << '\n';
    }

    Preprocessor_FillIncludes(ss, sourceCode, createInfo.pIncludeParentPath);

    return ss.str();
}


bool ShaderStage::GetCompilationStatus() const noexcept
{
    if (!IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Checking compile status by shader stage with invalid id: {}", m_stageID);
        return false;
    }

    GLint successStatus;
    glGetShaderiv(m_stageID, GL_COMPILE_STATUS, &successStatus);

    if (!successStatus) {
#if defined(ENG_LOGGING_ENABLED) 
        GLchar infoLog[512] = { 0 };
        glGetShaderInfoLog(m_stageID, sizeof(infoLog), nullptr, infoLog);

        ENG_LOG_GRAPHICS_API_ERROR("Shader stage (id: {}) compilation error: {}", m_stageID, infoLog);
#endif

        return false;
    }

    return true;
}


uint64_t ProgramUniform::Hash() const noexcept
{
    ds::HashBuilder builder;

    builder.AddValue(m_name);
    builder.AddValue(m_location);

#if defined(ENG_DEBUG)
    builder.AddValue(m_count);
    builder.AddValue(m_type);
#endif

    return builder.Value();
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
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");

    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_BOOL, 1);

    glProgramUniform1i(m_pOwner->GetRenderID(), pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformBool2(ds::StrID uniformName, bool x, bool y) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");

    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_BOOL, 2);

    glProgramUniform2i(m_pOwner->GetRenderID(), pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformBool3(ds::StrID uniformName, bool x, bool y, bool z) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_BOOL, 3);

    glProgramUniform3i(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformBool4(ds::StrID uniformName, bool x, bool y, bool z, bool w) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_BOOL, 4);

    glProgramUniform4i(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformInt(ds::StrID uniformName, int32_t value) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_INT, 1);

    glProgramUniform1i(m_pOwner->GetRenderID(), pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformInt2(ds::StrID uniformName, int32_t x, int32_t y) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_IVEC2, 1);

    glProgramUniform2i(m_pOwner->GetRenderID(), pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformInt3(ds::StrID uniformName, int32_t x, int32_t y, int32_t z) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_IVEC3, 1);

    glProgramUniform3i(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformInt4(ds::StrID uniformName, int32_t x, int32_t y, int32_t z, int32_t w) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_IVEC4, 1);

    glProgramUniform4i(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformUInt(ds::StrID uniformName, uint32_t value) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_UINT, 1);

    glProgramUniform1ui(m_pOwner->GetRenderID(), pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformUInt2(ds::StrID uniformName, uint32_t x, uint32_t y) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_UVEC2, 1);

    glProgramUniform2ui(m_pOwner->GetRenderID(), pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformUInt3(ds::StrID uniformName, uint32_t x, uint32_t y, uint32_t z) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_UVEC3, 1);

    glProgramUniform3ui(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformUInt4(ds::StrID uniformName, uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_UVEC4, 1);

    glProgramUniform4ui(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformFloat(ds::StrID uniformName, float value) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_FLOAT, 1);

    glProgramUniform1f(m_pOwner->GetRenderID(), pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformFloat2(ds::StrID uniformName, float x, float y) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_FVEC2, 1);

    glProgramUniform2f(m_pOwner->GetRenderID(), pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformFloat3(ds::StrID uniformName, float x, float y, float z) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_FVEC3, 1);

    glProgramUniform3f(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformFloat4(ds::StrID uniformName, float x, float y, float z, float w) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_FVEC4, 1);

    glProgramUniform4f(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformDouble(ds::StrID uniformName, double value) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_DOUBLE, 1);

    glProgramUniform1d(m_pOwner->GetRenderID(), pUniform->m_location, value);
}


void ProgramUniformStorage::SetUniformDouble2(ds::StrID uniformName, double x, double y) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_DVEC2, 1);

    glProgramUniform2d(m_pOwner->GetRenderID(), pUniform->m_location, x, y);
}


void ProgramUniformStorage::SetUniformDouble3(ds::StrID uniformName, double x, double y, double z) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_DVEC3, 1);

    glProgramUniform3d(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z);
}


void ProgramUniformStorage::SetUniformDouble4(ds::StrID uniformName, double x, double y, double z, double w) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_DVEC4, 1);

    glProgramUniform4d(m_pOwner->GetRenderID(), pUniform->m_location, x, y, z, w);
}


void ProgramUniformStorage::SetUniformFloat2x2(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT2X2, matrCount);

    glProgramUniformMatrix2fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat3x3(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT3X3, matrCount);

    glProgramUniformMatrix3fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat4x4(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT4X4, matrCount);

    glProgramUniformMatrix4fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat2x3(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT2X3, matrCount);

    glProgramUniformMatrix2x3fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat3x2(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT3X2, matrCount);

    glProgramUniformMatrix3x2fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat2x4(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT2X4, matrCount);

    glProgramUniformMatrix2x4fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat4x2(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT4X2, matrCount);

    glProgramUniformMatrix4x2fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat3x4(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT3X4, matrCount);

    glProgramUniformMatrix3x4fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::SetUniformFloat4x3(ds::StrID uniformName, const float *pMatrData, uint32_t matrCount, bool transpose) noexcept
{
    ENG_ASSERT_GRAPHICS_API(m_pOwner, "m_pOwner is nullptr");
    
    const ProgramUniform* pUniform = GetUniform(uniformName);
    CHECK_UNIFORM(pUniform, uniformName, ProgramUniform::TYPE_MAT4X3, matrCount);

    glProgramUniformMatrix4x3fv(m_pOwner->GetRenderID(), pUniform->m_location, matrCount, transpose, pMatrData);
}


void ProgramUniformStorage::Clear() noexcept
{
    m_uniforms.clear();
    m_pOwner = nullptr;
}


uint64_t ProgramUniformStorage::Hash() const noexcept
{
    ds::HashBuilder builder;
    
    for (const ProgramUniform& uniform : m_uniforms) {
        builder.AddValue(uniform);
    }

    builder.AddValue(*m_pOwner);
    
    return builder.Value();
}


ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_pUniformStorage, other.m_pUniformStorage);
    m_pUniformStorage->m_pOwner = this;
    std::swap(m_renderID, other.m_renderID);
}


ShaderProgram &ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_pUniformStorage, other.m_pUniformStorage);
    m_pUniformStorage->m_pOwner = this;
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


ProgramUniformStorage& ShaderProgram::GetUniformStorage() noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Trying to get uniform storage from invalid shader program");
    return *m_pUniformStorage;
}


uint64_t ShaderProgram::Hash() const noexcept
{
    ds::HashBuilder builder;

#if defined(ENG_DEBUG)
    builder.AddValue(m_dbgName);
#endif
    builder.AddValue(*m_pUniformStorage);
    builder.AddValue(m_renderID);
    
    return builder.Value();
}


bool ShaderProgram::Init(const ShaderProgramCreateInfo &createInfo) noexcept
{
    if (!createInfo.pStageCreateInfos || createInfo.stageCreateInfosCount == 0) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Shader program create info '{}' has invalid stages parametres", createInfo.dbgName.CStr());
        return false;
    }

    if (IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Recreating '{}' shader program", m_dbgName.CStr());
        Destroy();
    }

#if defined(ENG_DEBUG)
    m_dbgName = createInfo.dbgName;
#endif

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
        glAttachShader(m_renderID, shaderStages[i].m_stageID);
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
    if (IsValid()) {
    #if defined(ENG_DEBUG)
        m_dbgName = "";
    #endif

        m_pUniformStorage = nullptr;

        glDeleteProgram(m_renderID);
        m_renderID = 0;
    }
}


bool ShaderProgram::GetLinkingStatus() const noexcept
{
    if (!IsValidRenderID()) {
        ENG_LOG_GRAPHICS_API_ERROR("Invalid shader program '{}' id", m_dbgName.CStr());
        return false;
    }

    glValidateProgram(m_renderID);

    GLint linkSuccess;
    glGetProgramiv(m_renderID, GL_LINK_STATUS, &linkSuccess);

    if (!linkSuccess) {
#if defined(ENG_LOGGING_ENABLED)
        GLchar infoLog[512] = { 0 };
        glGetProgramInfoLog(m_renderID, sizeof(infoLog), nullptr, infoLog);

        ENG_LOG_GRAPHICS_API_ERROR("Shader program '{}' (id: {}) linking error: {}", m_dbgName.CStr(), m_renderID, infoLog);
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
    const uint64_t createInfoHash = amHash(createInfo);  

    const auto idIter = m_shaderProgramCreateInfoHashToIDMap.find(createInfoHash); 
    if (idIter != m_shaderProgramCreateInfoHashToIDMap.cend()) {
        ENG_LOG_GRAPHICS_API_WARN("Attempt to reregister '{}' shader program", createInfo.dbgName.CStr());
        return idIter->second;
    }

    ENG_ASSERT_GRAPHICS_API(m_shaderProgramsStorage.size() == m_uniformsStorage.size(), "m_shaderProgramsStorage.size() != m_uniformsStorage.size()");
    ENG_ASSERT_GRAPHICS_API(m_nextAllocatedID.m_id < m_shaderProgramsStorage.size() - 1, "Shader storage overflow");

    ShaderProgram program;

    if (!program.Init(createInfo)) {
        return ProgramID{};
    }

    ProgramID programID = AllocateProgramID();
    const uint64_t index = static_cast<uint64_t>(programID);
    
    m_shaderProgramCreateInfoHashToIDMap[createInfoHash] = programID;
    m_shaderProgramIDToCreateInfoHashVector[index] = createInfoHash;

    ProgramUniformStorage& uniformStorage = m_uniformsStorage[index];
    FillProgramUniformStorage(program, uniformStorage);

    program.m_pUniformStorage = &uniformStorage;
    m_shaderProgramsStorage[index] = std::move(program);

    return programID;
}


void ShaderManager::UnregisterShaderProgram(const ProgramID &id) noexcept
{
    if (!IsProgramIDValid(id)) {
        return;
    }

    const uint64_t index = static_cast<uint64_t>(id);

    uint64_t& createInfoHash = m_shaderProgramIDToCreateInfoHashVector[index];
    m_shaderProgramCreateInfoHashToIDMap.erase(createInfoHash);
    createInfoHash = 0;

    m_shaderProgramsStorage[index].Destroy();
    m_uniformsStorage[index].Clear();

    DeallocateProgramID(id);
}


ShaderProgram* ShaderManager::GetShaderProgramByID(const ProgramID& id) noexcept
{
    return IsProgramIDValid(id) ? &m_shaderProgramsStorage[static_cast<uint64_t>(id)] : nullptr;
}


const ProgramUniformStorage* ShaderManager::GetShaderProgramUniformStorageByID(const ProgramID &id) const noexcept
{
    return IsProgramIDValid(id) ? &m_uniformsStorage[static_cast<uint64_t>(id)] : nullptr;
}


bool ShaderManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_shaderProgramsStorage.resize(ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT);
    m_uniformsStorage.resize(ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT);
    m_shaderProgramIDToCreateInfoHashVector.resize(ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT);
    m_shaderProgramCreateInfoHashToIDMap.reserve(ENG_PREALLOCATED_SHADER_PROGRAMS_COUNT);
    
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
    storage.m_pOwner = &program;

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


ProgramID ShaderManager::AllocateProgramID() noexcept
{
    if (m_programIDFreeList.empty()) {
        ProgramID programID = m_nextAllocatedID;
        ++m_nextAllocatedID.m_id;

        return programID;
    }

    ProgramID programID = m_programIDFreeList.front();
    m_programIDFreeList.pop();
        
    return programID;
}


void ShaderManager::DeallocateProgramID(const ProgramID &id) noexcept
{
    if (id < m_nextAllocatedID) {
        m_programIDFreeList.push(id);
    }
}


bool ShaderManager::IsProgramIDValid(const ProgramID &id) const noexcept
{
    return id < m_nextAllocatedID && m_shaderProgramsStorage[static_cast<uint64_t>(id)].IsValid();
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

#if defined(ENG_DEBUG)
    builder.AddValue(programCreateInfo.dbgName);
#endif

    return builder.Value();
}


uint64_t amHash(const ShaderProgram &program) noexcept
{
    return program.Hash();
}


uint64_t amHash(const ProgramUniformStorage& programUniformStorage) noexcept
{
    return programUniformStorage.Hash();
}


uint64_t amHash(const ProgramUniform &programUniform) noexcept
{
    return programUniform.Hash();
}


bool engInitShaderManager() noexcept
{
    if (engIsShaderManagerInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("Shader manager is already initialized!");
        return true;
    }

    g_pShaderMng = std::unique_ptr<ShaderManager>(new ShaderManager);

    if (!g_pShaderMng) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to allocate memory for shader manager");
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

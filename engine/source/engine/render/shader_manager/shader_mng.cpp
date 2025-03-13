#include "pch.h"
#include "shader_mng.h"

#include "utils/data_structures/hash.h"
#include "utils/file/file.h"

#include "utils/debug/assertion.h"

#include "render/platform/OpenGL/opengl_driver.h"


static constexpr size_t ENG_MAX_SHADER_PROGRAMS_COUNT = 4096; // TODO: make it configurable
static constexpr size_t ENG_MAX_SHADER_INCLUDE_DEPTH = 128;   // TODO: make it configurable


static std::unique_ptr<ShaderManager> pShaderMngInst = nullptr;


static void Preprocessor_GetShaderVersionPosition(const std::string_view& sourceCode, ptrdiff_t& begin, ptrdiff_t& end) noexcept
{
    ENG_ASSERT(!sourceCode.empty(), "Source code is empty string view");

    static std::regex versionRegex("#version\\s*(\\d+) core");

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
            default: return GL_NONE;
        }
    }(createInfo.type);
    
    ENG_ASSERT_GRAPHICS_API(shaderStageGLType != GL_NONE, "Invalid ShaderStageType value: {}", static_cast<uint32_t>(createInfo.type));

    const std::string preprocessedSourceCode = PreprocessSourceCode(createInfo);
    if (preprocessedSourceCode.empty()) {
        ENG_LOG_WARN("Empty shader source code");
        return false;
    }

    if (IsValid()) {
        ENG_LOG_WARN("Recreation of shader stage: {}", m_stageID);
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
    ENG_ASSERT(createInfo.pSourceCode, "Source code is nullptr");

    std::stringstream ss;

    std::string_view sourceCode = createInfo.pSourceCode;
    
    ptrdiff_t versionPatternBeginPos, versionPatternEndPos;
    Preprocessor_GetShaderVersionPosition(sourceCode, versionPatternBeginPos, versionPatternEndPos);
    const std::streamsize versionPatternSize = versionPatternEndPos - versionPatternBeginPos;

    ss.write(sourceCode.data() + versionPatternBeginPos, versionPatternSize);

    sourceCode = sourceCode.data() + versionPatternBeginPos + versionPatternSize;

    for (size_t i = 0; i < createInfo.definesCount; ++i) {
        const char* pDefineStr = createInfo.pDefines[i];
        ENG_ASSERT(pDefineStr, "pDefineStr string is nullptr");
            
        ss << "#define " << pDefineStr << '\n';
    }

    Preprocessor_FillIncludes(ss, sourceCode, createInfo.pIncludeParentPath);

    return ss.str();
}


bool ShaderStage::GetCompilationStatus() const noexcept
{
    if (!IsValid()) {
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


ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif

    std::swap(m_ID, other.m_ID);
    std::swap(m_renderID, other.m_renderID);
}


ShaderProgram &ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif

    std::swap(m_ID, other.m_ID);
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void ShaderProgram::Bind() const noexcept
{
    ENG_ASSERT(IsValid(), "Attempt to bind invalid shader program");
    glUseProgram(m_renderID);
}


void ShaderProgram::SetLocalSrvBool(const ShaderResourceBindStruct<ShaderResourceType::TYPE_BOOL>& bind, bool value) noexcept
{
    ENG_ASSERT(IsValid(), "Sending boolean uniform to invalid shader program");
    glProgramUniform1i(m_renderID, bind.GetLocation(), value);
}


void ShaderProgram::SetLocalSrvInt(const ShaderResourceBindStruct<ShaderResourceType::TYPE_INT> &bind, int32_t value) noexcept
{
    ENG_ASSERT(IsValid(), "Sending int uniform to invalid shader program");
    glProgramUniform1i(m_renderID, bind.GetLocation(), value);
}


void ShaderProgram::SetLocalSrvUInt(const ShaderResourceBindStruct<ShaderResourceType::TYPE_UINT> &bind, uint32_t value) noexcept
{
    ENG_ASSERT(IsValid(), "Sending uint uniform to invalid shader program");
    glProgramUniform1ui(m_renderID, bind.GetLocation(), value);
}


void ShaderProgram::SetLocalSrvFloat(const ShaderResourceBindStruct<ShaderResourceType::TYPE_FLOAT> &bind, float value) noexcept
{
    ENG_ASSERT(IsValid(), "Sending float uniform to invalid shader program");
    glProgramUniform1f(m_renderID, bind.GetLocation(), value);
}


void ShaderProgram::SetLocalSrvDouble(const ShaderResourceBindStruct<ShaderResourceType::TYPE_DOUBLE> &bind, double value) noexcept
{
    ENG_ASSERT(IsValid(), "Sending double uniform to invalid shader program");
    glProgramUniform1d(m_renderID, bind.GetLocation(), value);
}


bool ShaderProgram::IsValid() const noexcept
{
    return m_ID.IsValid() && m_renderID != 0;
}


void ShaderProgram::SetDebugName(ds::StrID name) noexcept
{
#if defined(ENG_DEBUG)
    m_dbgName = name;
#endif
}


uint64_t ShaderProgram::Hash() const noexcept
{
    ds::HashBuilder builder;

    builder.AddValue(m_ID);
    builder.AddValue(m_renderID);

    return builder.Value();
}


ds::StrID ShaderProgram::GetDebugName() const noexcept
{
#if defined(ENG_DEBUG)
    return m_dbgName;
#else
    return "";
#endif
}


bool ShaderProgram::Create(const ShaderProgramCreateInfo &createInfo) noexcept
{
    ENG_ASSERT(!IsValid(), "Attempt to create already valid shader program: {}", m_dbgName.CStr());
    ENG_ASSERT(m_ID.IsValid(), "Shader program ID is invalid. You must initialize only programs which were returned by ShaderManager");
    
    ENG_ASSERT(createInfo.pStageCreateInfos && createInfo.stageCreateInfosCount > 0, 
        "Shader program create info '{}' has invalid stages parametres", m_dbgName.CStr());

    std::array<ShaderStage, static_cast<size_t>(ShaderStageType::COUNT)> shaderStages = {};
    for (size_t i = 0; i < createInfo.stageCreateInfosCount; ++i) {
        const ShaderStageCreateInfo* pStageCreateInfo = createInfo.pStageCreateInfos[i];
        
        ENG_ASSERT(pStageCreateInfo, "pStageCreateInfo is nullptr");
        
        if (!shaderStages[i].Init(*pStageCreateInfo)) {
            return false;
        }
    }

    m_renderID = glCreateProgram();
    
    for (size_t i = 0; i < createInfo.stageCreateInfosCount; ++i) {
        glAttachShader(m_renderID, shaderStages[i].m_stageID);
    }

    glLinkProgram(m_renderID);

    if (!GetLinkingStatus()) {
        Destroy();
        return false;
    }

    return true;
}


void ShaderProgram::Destroy() noexcept
{
    if (!IsValid()) {
        return;
    }

#if defined(ENG_DEBUG)
    m_dbgName = "_INVALID_";
#endif

    glDeleteProgram(m_renderID);
    m_renderID = 0;
}


bool ShaderProgram::GetLinkingStatus() const noexcept
{
    if (!IsValid()) {
        ENG_LOG_ERROR("Invalid shader program '{}' id", m_dbgName.CStr());
        return false;
    }

    glValidateProgram(m_renderID);

    GLint linkSuccess;
    glGetProgramiv(m_renderID, GL_LINK_STATUS, &linkSuccess);

    if (!linkSuccess) {
#if defined(ENG_LOGGING_ENABLED)
        GLchar infoLog[512] = { 0 };
        glGetProgramInfoLog(m_renderID, sizeof(infoLog), nullptr, infoLog);

        ENG_LOG_ERROR("Shader program '{}' (id: {}) linking error: {}", m_dbgName.CStr(), m_renderID, infoLog);
#endif

        return false;
    }

    return true;
}


ShaderManager &ShaderManager::GetInstance() noexcept
{
    ENG_ASSERT(engIsShaderManagerInitialized(), "Shader manager is not initialized");
    return *pShaderMngInst;
}


ShaderManager::~ShaderManager()
{
    Terminate();
}


ShaderProgram* ShaderManager::RegisterShaderProgram() noexcept
{
    const ProgramID programID = m_IDPool.Allocate();
    ENG_ASSERT(programID.Value() < m_shaderProgramsStorage.size(), "Shader storage overflow");
    
    ShaderProgram* pProgram = &m_shaderProgramsStorage[programID.Value()];

    ENG_ASSERT(!pProgram->IsValid(), "Valid shader program was returned during registration");

    pProgram->m_ID = programID;

    return pProgram;
}


void ShaderManager::UnregisterShaderProgram(ShaderProgram* pProgram) noexcept
{
    if (!pProgram) {
        return;
    }

    if (pProgram->IsValid()) {
        ENG_LOG_WARN("Unregistration of shader program \'{}\' while it's steel valid. Prefer to destroy shaders manually", pProgram->GetDebugName().CStr());
        pProgram->Destroy();
    }

    m_IDPool.Deallocate(pProgram->m_ID);
}


bool ShaderManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_shaderProgramsStorage.resize(ENG_MAX_SHADER_PROGRAMS_COUNT);
    m_IDPool.Reset();
    m_isInitialized = true;

    return true;
}


void ShaderManager::Terminate() noexcept
{
    m_shaderProgramsStorage.clear();
    m_IDPool.Reset();

    m_isInitialized = false;
}


bool ShaderManager::IsInitialized() const noexcept
{
    return m_isInitialized;
}


uint64_t amHash(const ShaderProgram& program) noexcept
{
    return program.Hash();
}


bool engInitShaderManager() noexcept
{
    if (engIsShaderManagerInitialized()) {
        ENG_LOG_WARN("Shader manager is already initialized!");
        return true;
    }

    pShaderMngInst = std::unique_ptr<ShaderManager>(new ShaderManager);

    if (!pShaderMngInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for shader manager");
        return false;
    }

    if (!pShaderMngInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized shader manager");
        return false;
    }

    return true;
}


void engTerminateShaderManager() noexcept
{
    pShaderMngInst = nullptr;
}


bool engIsShaderManagerInitialized() noexcept
{
    return pShaderMngInst && pShaderMngInst->IsInitialized();
}
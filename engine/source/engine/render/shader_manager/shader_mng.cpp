#include "pch.h"
#include "shader_mng.h"

#include "utils/data_structures/hash.h"
#include "utils/file/file.h"

#include "utils/debug/assertion.h"

#include "engine/render/platform/OpenGL/opengl_driver.h"


static constexpr size_t ENG_MAX_SHADER_PROGRAMS_COUNT = 4096; // TODO: make it configurable
static constexpr size_t ENG_MAX_UNIFORM_NAME_LENGTH = 128;    // TODO: make it configurable
static constexpr size_t ENG_MAX_SHADER_INCLUDE_DEPTH = 128;   // TODO: make it configurable


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


ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_renderID, other.m_renderID);
}


ShaderProgram &ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    Destroy();

#if defined(ENG_DEBUG)
    std::swap(m_dbgName, other.m_dbgName);
#endif
    std::swap(m_renderID, other.m_renderID);

    return *this;
}


void ShaderProgram::Bind() const noexcept
{
    ENG_ASSERT_GRAPHICS_API(IsValid(), "Trying to bind invalid shader program");
    glUseProgram(m_renderID);
}


void ShaderProgram::Unbind() const noexcept
{
    glUseProgram(0);
}


uint64_t ShaderProgram::Hash() const noexcept
{
    ds::HashBuilder builder;

#if defined(ENG_DEBUG)
    builder.AddValue(m_dbgName);
#endif
    builder.AddValue(m_renderID);
    
    return builder.Value();
}


bool ShaderProgram::Init(const ShaderProgramCreateInfo &createInfo) noexcept
{
    ENG_ASSERT_GRAPHICS_API(createInfo.pStageCreateInfos && createInfo.stageCreateInfosCount > 0, "Shader program create info '{}' has invalid stages parametres", createInfo.dbgName.CStr());

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

    ENG_ASSERT_GRAPHICS_API(m_nextAllocatedID < m_shaderProgramsStorage.size() - 1, "Shader storage overflow");

    ShaderProgram program;

    if (!program.Init(createInfo)) {
        return ProgramID{};
    }

    ProgramID programID = AllocateProgramID();
    const uint64_t index = programID;
    
    m_shaderProgramCreateInfoHashToIDMap[createInfoHash] = programID;
    m_shaderProgramIDToCreateInfoHashVector[index] = createInfoHash;

    m_shaderProgramsStorage[index] = std::move(program);

    return programID;
}


void ShaderManager::UnregisterShaderProgram(const ProgramID &id) noexcept
{
    if (!IsValidProgramID(id)) {
        return;
    }

    const uint64_t index = id;

    uint64_t& createInfoHash = m_shaderProgramIDToCreateInfoHashVector[index];
    m_shaderProgramCreateInfoHashToIDMap.erase(createInfoHash);
    createInfoHash = 0;

    m_shaderProgramsStorage[index].Destroy();

    DeallocateProgramID(id);
}


ShaderProgram* ShaderManager::GetShaderProgramByID(const ProgramID& id) noexcept
{
    return IsValidProgramID(id) ? &m_shaderProgramsStorage[id] : nullptr;
}


bool ShaderManager::IsValidProgramID(const ProgramID &id) const noexcept
{
    return id < m_nextAllocatedID && m_shaderProgramsStorage[id].IsValid();
}


bool ShaderManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_shaderProgramsStorage.resize(ENG_MAX_SHADER_PROGRAMS_COUNT);
    m_shaderProgramIDToCreateInfoHashVector.resize(ENG_MAX_SHADER_PROGRAMS_COUNT);
    m_shaderProgramCreateInfoHashToIDMap.reserve(ENG_MAX_SHADER_PROGRAMS_COUNT);
    
    m_isInitialized = true;

    return true;
}


void ShaderManager::Terminate() noexcept
{
    m_shaderProgramsStorage.clear();
    
    m_shaderProgramIDToCreateInfoHashVector.clear();
    m_shaderProgramCreateInfoHashToIDMap.clear();

    m_programIDFreeList.clear();

    m_nextAllocatedID = 0;

    m_isInitialized = false;
}


bool ShaderManager::IsInitialized() const noexcept
{
    return m_isInitialized;
}


ProgramID ShaderManager::AllocateProgramID() noexcept
{
    if (m_programIDFreeList.empty()) {
        ProgramID programID = m_nextAllocatedID;
        ++m_nextAllocatedID;

        return programID;
    }

    ProgramID programID = m_programIDFreeList.front();
    m_programIDFreeList.pop_front();
        
    return programID;
}


void ShaderManager::DeallocateProgramID(const ProgramID &ID) noexcept
{
    if (ID < m_nextAllocatedID && std::find(m_programIDFreeList.cbegin(), m_programIDFreeList.cend(), ID) == m_programIDFreeList.cend()) {
        m_programIDFreeList.emplace_back(ID);
    }
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

    builder.AddValue(programCreateInfo.dbgName);

    return builder.Value();
}


uint64_t amHash(const ShaderProgram &program) noexcept
{
    return program.Hash();
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
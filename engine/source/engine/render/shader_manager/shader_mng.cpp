#include "pch.h"
#include "shader_mng.h"

#include "utils/debug/assertion.h"

#include <glad/glad.h>


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
    ShaderStage() = default;
    ~ShaderStage() { Destroy(); }

    bool Create(const ShaderStageCreateInfo& createInfo) noexcept;
    void Destroy() noexcept { glDeleteShader(m_id); m_id = 0; }

    bool IsValid() const noexcept { return m_id != 0; }

private:
    bool GetCompilationStatus() const noexcept;

private:
    uint32_t m_id = 0;
};


class ShaderProgram
{
    friend class ShaderManager;

public:
    ShaderProgram() = default;
    ~ShaderProgram() { Destroy(); }

    bool Create(const ShaderProgramCreateInfo& createInfo) noexcept;
    void Destroy() noexcept { glDeleteProgram(m_id); m_id = 0; }

    bool IsValid() const noexcept { return m_id != 0; }

private:
    bool GetLinkingStatus() const noexcept;

private:
    uint32_t m_id = 0;
};


bool ShaderStage::Create(const ShaderStageCreateInfo& createInfo) noexcept
{
    if (!createInfo.pSourceCode || createInfo.codeSize == 0) {
        ENG_ASSERT_GRAPHICS_API_FAIL("pSourceCode is nullptr or zero sized");
        return false;
    }

    const GLenum shaderStageGLType = ToOpenGLNativeShaderStageType(createInfo.type);
    if (shaderStageGLType == GL_NONE) {
        return false;
    }

    if (IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Calling ShaderStage::Create(const ShaderStageCreateInfo&) by shader stage with valid id: {}", m_id);
        Destroy();
    }

    m_id = glCreateShader(shaderStageGLType);

    const int32_t codeSize = createInfo.codeSize;
    ENG_ASSERT_GRAPHICS_API(codeSize > 0, "codeSize value overflow");

    glShaderSource(m_id, 1, &createInfo.pSourceCode, &codeSize);

    glCompileShader(m_id);

    const bool compilationSuccess = GetCompilationStatus();
    if (!compilationSuccess) {
        Destroy();
    }

    return compilationSuccess;
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


bool ShaderProgram::Create(const ShaderProgramCreateInfo &createInfo) noexcept
{
    if (!createInfo.pStages || createInfo.stagesCount == 0) {
        ENG_ASSERT_GRAPHICS_API_FAIL("pStages is nullptr or zero sized");
        return false;
    }

    if (IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Calling ShaderProgram::Create(const ShaderProgramCreateInfo&) by shader program with valid id: {}", m_id);
        Destroy();
    }

    std::array<ShaderStage, static_cast<size_t>(ShaderStageType::COUNT)> shaderStages = {};
    for (size_t i = 0; i < createInfo.stagesCount; ++i) {
        if (!shaderStages[i].Create(createInfo.pStages[i])) {
            return false;
        }
    }

    m_id = glCreateProgram();
    
    for (size_t i = 0; i < createInfo.stagesCount; ++i) {
        glAttachShader(m_id, shaderStages[i].m_id);
    }

    glLinkProgram(m_id);

    const bool linkingSuccess = GetLinkingStatus();
    if (!linkingSuccess) {
        Destroy();
    }

    return linkingSuccess;
}


bool ShaderProgram::GetLinkingStatus() const noexcept
{
    if (!IsValid()) {
        ENG_LOG_GRAPHICS_API_WARN("Checking compile status by shader stage with invalid id: {}", m_id);
        return false;
    }

    glValidateProgram(m_id);

    GLint linkSuccess;
    glGetProgramiv(m_id, GL_LINK_STATUS, &linkSuccess);

    if (!linkSuccess) {
#if defined(ENG_LOGGING_ENABLED)
        GLchar infoLog[512] = { 0 };
        glGetProgramInfoLog(m_id, sizeof(infoLog), nullptr, infoLog);

        ENG_LOG_GRAPHICS_API_ERROR("Shader program (id: {}) linking error: {}", m_id, infoLog);
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


bool ShaderManager::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_isInitialized = true;
    
    return true;
}


void ShaderManager::Terminate() noexcept
{
    m_isInitialized = false;
}


bool ShaderManager::IsInitialized() const noexcept
{
    return m_isInitialized;
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

#include "pch.h"
#include "render_system.h"

#include "engine/window_system/window.h"

#include "utils/file/file.h"

#include "utils/debug/assertion.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


static std::unique_ptr<RenderSystem> g_pRenderSystem = nullptr;


static void GLAPIENTRY OpenGLMessageCallback(ENG_MAYBE_UNUSED GLenum source, ENG_MAYBE_UNUSED GLenum type, ENG_MAYBE_UNUSED GLuint id,
    ENG_MAYBE_UNUSED GLenum severity, ENG_MAYBE_UNUSED GLsizei length, const GLchar* message, ENG_MAYBE_UNUSED const void* userParam)
{
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        ENG_ASSERT_GRAPHICS_API_FAIL(message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        ENG_LOG_GRAPHICS_API_WARN(message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        ENG_LOG_GRAPHICS_API_INFO(message);
        break;
    }
}


RenderSystem& RenderSystem::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsRenderSystemInitialized(), "Render system is not initialized");
    return *g_pRenderSystem;
}


void RenderSystem::BeginFrame() noexcept
{
    glClear(GL_COLOR_BUFFER_BIT);
}


void RenderSystem::EndFrame() noexcept
{

}


void RenderSystem::RunDepthPrepass() noexcept
{

}


void RenderSystem::RunGBufferPass() noexcept
{

}


void RenderSystem::RunColorPass() noexcept
{
    static bool isInitialized = false;
    static ShaderProgram* pProgram = nullptr;
    static uint32_t vao = 0;

    if (!isInitialized) {
        ShaderStageCreateInfo vsStageCreateInfo = {};

        vsStageCreateInfo.type = ShaderStageType::VERTEX;

        const std::vector<char> vsSourceCode = ReadTextFile("D:\\Studies\\Graphics\\random-graphics\\engine\\source\\shaders\\base\\base.vs");
        vsStageCreateInfo.pSourceCode = vsSourceCode.data();
        vsStageCreateInfo.codeSize = vsSourceCode.size();

        vsStageCreateInfo.pDefines = nullptr;
        vsStageCreateInfo.definesCount = 0;


        ShaderStageCreateInfo psStageCreateInfo = {};

        psStageCreateInfo.type = ShaderStageType::PIXEL;

        const std::vector<char> psSourceCode = ReadTextFile("D:\\Studies\\Graphics\\random-graphics\\engine\\source\\shaders\\base\\base.fs");
        psStageCreateInfo.pSourceCode = psSourceCode.data();
        psStageCreateInfo.codeSize = psSourceCode.size();

        psStageCreateInfo.pDefines = nullptr;
        psStageCreateInfo.definesCount = 0;


        const ShaderStageCreateInfo* stages[] = { &vsStageCreateInfo, &psStageCreateInfo };

        ShaderProgramCreateInfo programCreateInfo = {};
        programCreateInfo.pStageCreateInfos = stages;
        programCreateInfo.stageCreateInfosCount = _countof(stages);

        ShaderID shaderID = ShaderManager::GetInstance().RegisterShaderProgram(programCreateInfo);
        ENG_ASSERT_GRAPHICS_API(shaderID.IsValid(), "Invalid Shader ID");

        pProgram = ShaderManager::GetInstance().GetShaderProgramByID(shaderID);

        glCreateVertexArrays(1, &vao);

        isInitialized = true;
    }

    pProgram->Bind();
    glBindVertexArray(vao);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
}


void RenderSystem::RunPostprocessingPass() noexcept
{

}


RenderSystem::~RenderSystem()
{
    Terminate();
}


bool RenderSystem::Init() noexcept
{
#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
#endif

    if (!engInitShaderManager()) {
        return false;
    }

    return true;
}
    
    
void RenderSystem::Terminate() noexcept
{
    engTerminateShaderManager();
}


bool RenderSystem::IsInitialized() const noexcept
{
    return engIsShaderManagerInitialized();
}


bool engInitRenderSystem() noexcept
{
    if (engIsRenderSystemInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("Render system is already initialized!");
        return true;
    }

    if (!engIsWindowSystemInitialized()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Window system must be initialized before render system");
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialize GLAD");
        return false;
    }

    g_pRenderSystem = std::unique_ptr<RenderSystem>(new RenderSystem);
    if (!g_pRenderSystem) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to create render system");
        return false;
    }

    if (!g_pRenderSystem->Init()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialized render system");
        return false;
    }

    return true;
}


void engTerminateRenderSystem() noexcept
{
    g_pRenderSystem = nullptr;
}


bool engIsRenderSystemInitialized() noexcept
{
    return g_pRenderSystem && g_pRenderSystem->IsInitialized();
}
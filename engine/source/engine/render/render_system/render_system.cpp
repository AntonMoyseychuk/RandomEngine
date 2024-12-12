#include "pch.h"
#include "render_system.h"

#include "engine/window_system/window.h"

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

    return true;
}
    
    
void RenderSystem::Terminate() noexcept
{
}


bool RenderSystem::IsInitialized() const noexcept
{
    return true;
}


bool engInitRenderSystem() noexcept
{
    if (g_pRenderSystem) {
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
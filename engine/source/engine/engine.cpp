#include "pch.h"

#include "engine/engine.h"
#include "engine/event_system/event_dispatcher.h"

#include "engine/render/render_system/render_system.h"

#include "utils/debug/assertion.h"


#define ENG_CHECK_WINDOW_INITIALIZATION(pWindow) ENG_ASSERT(pWindow && pWindow->IsInitialized(), "Window is nullptr or not initialized");
#define ENG_CHECK_REND_SYS_INITIALIZATION()      ENG_ASSERT(engIsRenderSystemInitialized(), "Render system is not initialized");


static std::unique_ptr<Engine> g_pEngine = nullptr;


Engine& Engine::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsEngineInitialized(), "Engine is not initialized");
    return *g_pEngine;
}


bool Engine::Init(const char* title, uint32_t width, uint32_t height) noexcept
{
    if (engIsEngineInitialized()) {
        ENG_LOG_WARN("Engine is already initialized!");
        return true;
    }

    g_pEngine = std::unique_ptr<Engine>(new Engine(title, width, height));

    if (!g_pEngine) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to allocate memory for engine");
        return false;
    }

    if (!g_pEngine->IsInitialized()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialize engine");
        return false;
    }

    return true;
}


void Engine::Terminate() noexcept
{
    g_pEngine = nullptr;
}


Engine::~Engine()
{
    WindowSystem& windowSys = WindowSystem::GetInstance();
    windowSys.DestroyWindow(WINDOW_TAG_MAIN);
    m_pMainWindow = nullptr;

    engTerminateRenderSystem();
    engTerminateWindowSystem();    
    engTerminateLogSystem();
}


void Engine::Update() noexcept
{
    ENG_CHECK_WINDOW_INITIALIZATION(m_pMainWindow);
    
    m_pMainWindow->PollEvents();
}


void Engine::BeginFrame() noexcept
{
    ENG_CHECK_REND_SYS_INITIALIZATION();

    RenderSystem::GetInstance().BeginFrame();
}


void Engine::EndFrame() noexcept
{
    ENG_CHECK_WINDOW_INITIALIZATION(m_pMainWindow);
    ENG_CHECK_REND_SYS_INITIALIZATION();

    RenderSystem::GetInstance().EndFrame();

    m_pMainWindow->SwapBuffers();
}


void Engine::RenderFrame() noexcept
{
    ENG_CHECK_WINDOW_INITIALIZATION(m_pMainWindow);
    ENG_CHECK_REND_SYS_INITIALIZATION();

    if (m_pMainWindow->IsMinimized()) {
        return;
    }

    RenderSystem& renderSys = RenderSystem::GetInstance();

    // Depth Prepass
    renderSys.RunDepthPrepass();

    // GBuffer Pass
    renderSys.RunGBufferPass();

    // Forward + Lighting Pass
    renderSys.RunColorPass();

    // Post Process
    renderSys.RunPostprocessingPass();
}


bool Engine::IsInitialized() const noexcept
{
    return m_isInitialized;
}


Window& Engine::GetMainWindow() noexcept
{
    ENG_ASSERT(engIsWindowSystemInitialized(), "Window system is not initialized");
    return *m_pMainWindow;
}


Engine::Engine(const char *title, uint32_t width, uint32_t height)
{
    engInitLogSystem();

    if (!engInitWindowSystem()) {
        return;
    }

    WindowSystem& windowSys = WindowSystem::GetInstance();

    WindowCreateInfo mainWindowCreateInfo = {};
    mainWindowCreateInfo.pTitle = title;
    mainWindowCreateInfo.width = width;
    mainWindowCreateInfo.height = height;
    m_pMainWindow = windowSys.CreateWindow(WINDOW_TAG_MAIN, mainWindowCreateInfo);

    if (!(m_pMainWindow && m_pMainWindow->IsInitialized())) {
        return;
    }

    if (!engInitRenderSystem()) {
        return;
    }

    // Notify all subscribed systems to resized their resources
    EventDispatcher::GetInstance().Notify<EventFramebufferResized>(m_pMainWindow->GetFramebufferWidth(), m_pMainWindow->GetFramebufferHeight());

    m_isInitialized = true;

    m_pMainWindow->ShowWindow();
}


bool engIsEngineInitialized() noexcept
{
    return g_pEngine && g_pEngine->IsInitialized();
}
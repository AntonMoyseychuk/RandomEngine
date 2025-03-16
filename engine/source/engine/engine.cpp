#include "pch.h"

#include "engine/engine.h"
#include "core/event_system/event_dispatcher.h"
#include "core/window_system/window_system.h"

#include "render/render_system/render_system.h"
#include "core/camera/camera_manager.h"

#include "utils/debug/assertion.h"


#define ENG_CHECK_REND_SYS_INITIALIZATION() ENG_ASSERT(engIsRenderSystemInitialized(), "Render system is not initialized");


static std::unique_ptr<Engine> pEngineInst = nullptr;
static Window* pMainWindowInst = nullptr;


Engine& Engine::GetInstance() noexcept
{
    ENG_ASSERT_GRAPHICS_API(engIsEngineInitialized(), "Engine is not initialized");
    return *pEngineInst;
}


bool Engine::Init(const char* title, uint32_t width, uint32_t height, bool enableVSync) noexcept
{
    if (engIsEngineInitialized()) {
        ENG_LOG_WARN("Engine is already initialized!");
        return true;
    }

    pEngineInst = std::unique_ptr<Engine>(new Engine(title, width, height, enableVSync));

    if (!pEngineInst) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to allocate memory for engine");
        return false;
    }

    if (!pEngineInst->IsInitialized()) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialize engine");
        return false;
    }

    return true;
}


void Engine::Terminate() noexcept
{
    pEngineInst = nullptr;
}


Engine::~Engine()
{
    pMainWindowInst = nullptr;
    engTerminateRenderSystem();
    engTerminateCameraManager();
    engTerminateWindowSystem();    
    engTerminateLogSystem();
}


void Engine::Update() noexcept
{
    pMainWindowInst->Update();
    CameraManager::GetInstance().Update(1.f);
}


void Engine::BeginFrame() noexcept
{
    RenderSystem::GetInstance().BeginFrame();
}


void Engine::EndFrame() noexcept
{
    RenderSystem::GetInstance().EndFrame();
    pMainWindowInst->SwapBuffers();
}


void Engine::RenderFrame() noexcept
{
    if (pMainWindowInst->IsMinimized()) {
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


bool Engine::IsRunning() const noexcept
{
    return pMainWindowInst && !pMainWindowInst->IsClosed();
}


bool Engine::IsInitialized() const noexcept
{
    return m_isInitialized;
}


Engine::Engine(const char* title, uint32_t width, uint32_t height, bool enableVSync)
{
    engInitLogSystem();

    if (!engInitWindowSystem()) {
        return;
    }

    WindowCreateInfo mainWindowCreateInfo = {};
    mainWindowCreateInfo.pTitle = title;
    mainWindowCreateInfo.width = width;
    mainWindowCreateInfo.height = height;
    mainWindowCreateInfo.enableVSync = enableVSync;

    pMainWindowInst = WindowSystem::GetInstance().CreateWindow(WINDOW_TAG_MAIN, mainWindowCreateInfo);

    if (!(pMainWindowInst && pMainWindowInst->IsInitialized())) {
        return;
    }

    if (!engInitCameraManager()) {
        return;
    }

    if (!engInitRenderSystem()) {
        return;
    }

    // Notify all subscribed systems to resized their resources
    es::EventDispatcher::GetInstance().Notify<EventFramebufferResized>(pMainWindowInst->GetFramebufferWidth(), pMainWindowInst->GetFramebufferHeight());

    m_isInitialized = true;

    pMainWindowInst->Show();
}


bool engIsEngineInitialized() noexcept
{
    return pEngineInst && pEngineInst->IsInitialized();
}
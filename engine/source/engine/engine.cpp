#include "pch.h"

#include "engine/engine.h"

#include "engine/render/render_system/render_system.h"

#include "utils/debug/assertion.h"


#define ENG_CHECK_WINDOW_INITIALIZATION(pWindow) ENG_ASSERT(pWindow && pWindow->IsInitialized(), "Window is nullptr or not initialized");
#define ENG_CHECK_REND_SYS_INITIALIZATION()      ENG_ASSERT(engIsRenderSystemInitialized(), "Render system is not initialized");


bool Engine::Init(const char* title, uint32_t width, uint32_t height) noexcept
{
    engInitLogSystem();

    if (IsInitialized()) {
        ENG_LOG_WARN("Engine is already initialized!");
        return true;
    }

    if (!engInitWindowSystem()) {
        return false;
    }

    m_pWindow = std::make_unique<Window>(title, width, height);

    if (!(m_pWindow && m_pWindow->IsInitialized())) {
        return false;
    }

    if (!engInitRenderSystem()) {
        return false;
    }

    m_pWindow->ShowWindow();

    return true;
}


void Engine::Terminate() noexcept
{
    m_pWindow = nullptr;

    engTerminateRenderSystem();
    engTerminateWindowSystem();    
    engTerminateLogSystem();
}


void Engine::Update() noexcept
{
    ENG_CHECK_WINDOW_INITIALIZATION(m_pWindow);
    
    m_pWindow->ProcessEvents();
}


void Engine::BeginFrame() noexcept
{
    ENG_CHECK_REND_SYS_INITIALIZATION();

    RenderSystem::GetInstance().BeginFrame();
}


void Engine::EndFrame() noexcept
{
    ENG_CHECK_WINDOW_INITIALIZATION(m_pWindow);
    ENG_CHECK_REND_SYS_INITIALIZATION();

    RenderSystem::GetInstance().EndFrame();

    m_pWindow->SwapBuffers();
}


void Engine::RenderFrame() noexcept
{
    ENG_CHECK_REND_SYS_INITIALIZATION();

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
    return m_pWindow && m_pWindow->IsInitialized() 
        && engIsRenderSystemInitialized();
}


Window &Engine::GetWindow() noexcept
{
    ENG_ASSERT(m_pWindow && m_pWindow->IsInitialized(), "Invalid nullptr or is not initialized");
    return *m_pWindow;
}
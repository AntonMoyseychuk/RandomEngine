#include "pch.h"

#include "engine/engine.h"

#include "utils/debug/assertion.h"


bool Engine::Init(const char* title, uint32_t width, uint32_t height) noexcept
{
    engInitLogSystem();

    if (IsInitialized()) {
        ENG_LOG_WARN("Engine is already initialized!");
        return true;
    }

    if (!engInitWindowSystem()) {
        ENG_ASSERT_WINDOW_FAIL("Window system initialization failed");
        return false;
    }

    m_pWindow = std::make_unique<Window>(title, width, height);

    if (!m_pWindow || !m_pWindow->IsInitialized()) {
        return false;
    }

    m_pWindow->ShowWindow();

    return true;
}


void Engine::Terminate() noexcept
{
    m_pWindow = nullptr;

    engTerminateWindowSystem();
    
    engTerminateLogSystem();
}


void Engine::Update() noexcept
{
    m_pWindow->ProcessEvents();

    if (m_pWindow->IsClosed()) {
        return;
    }

    m_pWindow->SwapBuffers();
}


bool Engine::IsInitialized() const noexcept
{
    return m_pWindow && m_pWindow->IsInitialized();
}


Window &Engine::GetWindow() noexcept
{
    ENG_ASSERT(m_pWindow && m_pWindow->IsInitialized(), "Invalid nullptr or is not initialized");
    return *m_pWindow;
}
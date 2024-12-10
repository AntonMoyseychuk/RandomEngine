#include "pch.h"

#include "application.h"


bool Application::Init(const char* title, uint32_t width, uint32_t height) noexcept
{
    if (IsInitialized()) {
        return true;
    }

    m_pEngine = std::make_unique<Engine>();
    return m_pEngine->Init(title, width, height);
}


void Application::Terminate() noexcept
{
    m_pEngine = nullptr;
}


void Application::Run() noexcept
{
    Window& window = m_pEngine->GetWindow();

    while(!window.IsClosed()) {
        if (!m_pEngine->BeginFrame()) {
            continue;
        }
        
        m_pEngine->RenderFrame();
        m_pEngine->EndFrame();
    }
}


bool Application::IsInitialized() const noexcept
{
    return m_pEngine && m_pEngine->IsInitialized();
}
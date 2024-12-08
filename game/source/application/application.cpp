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
    while(true) {
        m_pEngine->Update();
    }
}


bool Application::IsInitialized() const noexcept
{
    return m_pEngine && m_pEngine->IsInitialized();
}
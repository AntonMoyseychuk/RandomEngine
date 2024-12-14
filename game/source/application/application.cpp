#include "pch.h"

#include "application.h"


bool Application::Init(const char* title, uint32_t width, uint32_t height) noexcept
{
    if (IsInitialized()) {
        return true;
    }

    if (!Engine::Init(title, width, height)) {
        return false;
    }

    return true;
}


void Application::Terminate() noexcept
{
    Engine::Terminate();
}


void Application::Run() noexcept
{
    Engine& engine = Engine::GetInstance();
    Window& window = engine.GetWindow();

    while(!window.IsClosed()) {
        engine.Update(); // In other thread may be ?...

        engine.BeginFrame();
        engine.RenderFrame();
        engine.EndFrame();
    }
}


bool Application::IsInitialized() const noexcept
{
    return engIsEngineInitialized();
}
#pragma once

#include "window_system/window.h"

#include <memory>
#include <cstdint>


class Engine
{
public:
    Engine() = default;

    bool Init(const char* title, uint32_t width, uint32_t height) noexcept;
    void Terminate() noexcept;

    void Update() noexcept;

    void BeginFrame() noexcept;
    void EndFrame() noexcept;
    
    void RenderFrame() noexcept;

    bool IsInitialized() const noexcept;

    Window& GetWindow() noexcept;

private:
    std::unique_ptr<Window> m_pWindow = nullptr;
};
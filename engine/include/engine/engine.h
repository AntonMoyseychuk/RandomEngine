#pragma once

#include "window_system/window.h"
#include "timer/timer.h"

#include <memory>
#include <cstdint>


class Engine
{
public:
    static Engine& GetInstance() noexcept;

    static bool Init(const char* title, uint32_t width, uint32_t height) noexcept;
    static void Terminate() noexcept;

public:
    Engine(const Engine& other) = delete;
    Engine& operator=(const Engine& other) = delete;
    
    ~Engine();

    void Update() noexcept;

    void BeginFrame() noexcept;
    void EndFrame() noexcept;
    
    void RenderFrame() noexcept;

    bool IsInitialized() const noexcept;

    Window& GetWindow() noexcept;
    Timer& GetTimer() noexcept;

private:
    Engine(const char* title, uint32_t width, uint32_t height);

    Engine(Engine&& other) noexcept = default;
    Engine& operator=(Engine&& other) noexcept = default;

private:
    Timer m_timer;
    std::unique_ptr<Window> m_pWindow = nullptr;

    bool m_isInitialized = false;
};


bool engIsEngineInitialized() noexcept;
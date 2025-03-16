#pragma once

#include <memory>
#include <cstdint>


class Engine
{
public:
    static Engine& GetInstance() noexcept;

    static bool Init(const char* title, uint32_t width, uint32_t height, bool enableVSync) noexcept;
    static void Terminate() noexcept;

public:
    Engine(const Engine& other) = delete;
    Engine& operator=(const Engine& other) = delete;
    
    ~Engine();

    void Update() noexcept;

    void BeginFrame() noexcept;
    void EndFrame() noexcept;
    
    void RenderFrame() noexcept;

    bool IsRunning() const noexcept;
    bool IsInitialized() const noexcept;
    
private:
    Engine(const char* title, uint32_t width, uint32_t height, bool enableVSync);

    Engine(Engine&& other) noexcept = default;
    Engine& operator=(Engine&& other) noexcept = default;

private:
    bool m_isInitialized = false;
};


bool engIsEngineInitialized() noexcept;
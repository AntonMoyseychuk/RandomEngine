#pragma once

#include "engine/engine.h"

#include <memory>


class Application
{
public:
    Application() = default;

    bool Init(const char* title, uint32_t width, uint32_t height) noexcept;
    void Terminate() noexcept;

    void Run() noexcept;

    bool IsInitialized() const noexcept;

private:
    std::unique_ptr<Engine> m_pEngine = nullptr;
};
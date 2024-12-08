#pragma once

#include <cstdint>


class Engine
{
public:
    Engine() = default;

    bool Init(const char* title, uint32_t width, uint32_t height) noexcept;
    void Terminate() noexcept;

    void Update() noexcept;

    bool IsInitialized() const noexcept;
};
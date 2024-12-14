#pragma once

#include "input.h"

#include <cstdint>


struct GLFWwindow;


class Window
{
    friend class Input;
public:
    Window(const char* title, uint32_t width, uint32_t height);
    ~Window();

    void ProcessEvents() noexcept;
    void SwapBuffers() noexcept;

    void ShowWindow() noexcept;
    void HideWindow() noexcept;
    
    void Close() noexcept;

    uint32_t GetWidth() const noexcept;
    uint32_t GetHeight() const noexcept;

    const char* GetTitle() const noexcept;
    void SetTitle(const char* title) noexcept;

    bool IsClosed() const noexcept;

    bool IsWindowInitialized() const noexcept { return m_pWindow != nullptr; }
    bool IsInputSysInitialized() const noexcept { return m_input.IsInitialized(); }
    bool IsInitialized() const noexcept { return IsWindowInitialized() && IsInputSysInitialized(); }

    Input& GetInput() noexcept { return m_input; }

private:
    Input m_input;

    GLFWwindow* m_pWindow = nullptr;
};


bool engInitWindowSystem() noexcept;
void engTerminateWindowSystem() noexcept;

bool engIsWindowSystemInitialized() noexcept;
#pragma once

#include "input.h"

#include <cstdint>
#include <bitset>


struct EventWindowMinimized
{
};


struct EventWindowMaximized
{
};


struct EventWindowSizeRestored
{
};


struct EventWindowClosed
{
};


struct EventWindowFocused
{
};


struct EventWindowUnfocused
{
};


struct EventWindowPositionChanged
{
public:
    EventWindowPositionChanged(int32_t x, int32_t y)
        : m_xpos(x), m_ypos(y) {}

    int32_t GetX() const noexcept { return m_xpos; }
    int32_t GetY() const noexcept { return m_ypos; }

private:
    int32_t m_xpos;
    int32_t m_ypos;
};


struct EventWindowResized
{
public:
    EventWindowResized(int32_t width, int32_t height)
        : m_width(width), m_height(height) {}

    int32_t GetWidth() const noexcept { return m_width; }
    int32_t GetHeight() const noexcept { return m_height; }

private:
    int32_t m_width;
    int32_t m_height;
};


struct EventFramebufferResized
{
public:
    EventFramebufferResized(int32_t width, int32_t height)
        : m_width(width), m_height(height) {}

    int32_t GetWidth() const noexcept { return m_width; }
    int32_t GetHeight() const noexcept { return m_height; }

private:
    int32_t m_width;
    int32_t m_height;
};


struct GLFWwindow;


class Window
{
    friend class Input;
private:
    enum WindowState : uint32_t
    {
        STATE_CLOSED,
        STATE_FOCUSED,
        STATE_MAXIMIZED,
        STATE_MINIMIZED,

        STATE_COUNT
    };

public:
    Window(const char* title, uint32_t width, uint32_t height);
    ~Window();

    void PollEvents() noexcept;
    void SwapBuffers() noexcept;

    void ShowWindow() noexcept;
    void HideWindow() noexcept;

    uint32_t GetWidth() const noexcept { return m_windowWidth; }
    uint32_t GetHeight() const noexcept { return m_windowHeight; }
    
    uint32_t GetFramebufferWidth() const noexcept { return m_framebufferWidth; }
    uint32_t GetFramebufferHeight() const noexcept { return m_framebufferHeight; }

    const char* GetTitle() const noexcept;
    void SetTitle(const char* title) noexcept;

    bool IsClosed() const noexcept { return m_state.test(STATE_CLOSED); }
    bool IsFocused() const noexcept { return m_state.test(STATE_FOCUSED); }
    bool IsMaximized() const noexcept { return m_state.test(STATE_MAXIMIZED); }
    bool IsMinimized() const noexcept { return m_state.test(STATE_MINIMIZED); }
    bool IsVisible() const noexcept;

    bool IsWindowInitialized() const noexcept { return m_pWindow != nullptr; }
    bool IsInputSysInitialized() const noexcept { return m_input.IsInitialized(); }
    bool IsInitialized() const noexcept { return IsWindowInitialized() && IsInputSysInitialized(); }

    Input& GetInput() noexcept { return m_input; }

private:
    Input m_input;

    GLFWwindow* m_pWindow = nullptr;

    uint32_t m_windowWidth = 0;
    uint32_t m_windowHeight = 0;

    uint32_t m_framebufferWidth = 0;
    uint32_t m_framebufferHeight = 0;

    std::bitset<STATE_COUNT> m_state = STATE_CLOSED;
};


bool engInitWindowSystem() noexcept;
void engTerminateWindowSystem() noexcept;

bool engIsWindowSystemInitialized() noexcept;
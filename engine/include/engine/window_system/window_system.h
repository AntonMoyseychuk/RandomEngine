#pragma once

#include "window_system_events.h"

#include <bitset>
#include <array>


enum class KeyboardKey : uint8_t
{
    KEY_SPACE,
    KEY_APOSTROPHE, /* ' */
    KEY_COMMA,      /* , */
    KEY_MINUS,      /* - */
    KEY_PERIOD,     /* . */
    KEY_SLASH,      /* / */
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,  /* ; */
    KEY_EQUAL,      /* = */
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFT_BRACKET,  /* [ */
    KEY_BACKSLASH,     /* \ */
    KEY_RIGHT_BRACKET, /* ] */
    KEY_GRAVE_ACCENT,  /* ` */
    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,
    KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_ALT,
    KEY_RIGHT_SUPER,
    KEY_MENU,

    KEY_COUNT
};


enum class MouseButton : uint8_t
{
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,

    BUTTON_COUNT
};


enum class KeyState : uint8_t
{
    STATE_RELEASED,
    STATE_PRESSED,
    STATE_HOLD,

    STATE_COUNT
};

enum class MouseButtonState : uint8_t
{
    STATE_RELEASED,
    STATE_PRESSED,
    STATE_HOLD,

    STATE_COUNT
};


struct CursorPositon
{
    float x;
    float y;
};


class Input
{
    friend class Window;

public:
    Input() = default;

    KeyState GetKeyState(KeyboardKey key) const noexcept;
    MouseButtonState GetMouseButtonState(MouseButton button) const noexcept;

    bool IsKeyPressed(KeyboardKey key) const noexcept { return GetKeyState(key) == KeyState::STATE_PRESSED; }
    bool IsKeyReleased(KeyboardKey key) const noexcept { return GetKeyState(key) == KeyState::STATE_RELEASED; }
    bool IsKeyHold(KeyboardKey key) const noexcept { return GetKeyState(key) == KeyState::STATE_HOLD; }
    bool IsKeyPressedOrHold(KeyboardKey key) const noexcept { return IsKeyPressed(key) || IsKeyHold(key); }

    bool IsMouseButtonPressed(MouseButton button) const noexcept { return GetMouseButtonState(button) == MouseButtonState::STATE_PRESSED; }
    bool IsMouseButtonReleased(MouseButton button) const noexcept { return GetMouseButtonState(button) == MouseButtonState::STATE_RELEASED; }
    bool IsMouseButtonHold(MouseButton button) const noexcept { return GetMouseButtonState(button) == MouseButtonState::STATE_HOLD; }
    bool IsMouseButtonPressedOrHold(MouseButton button) const noexcept { return IsMouseButtonPressed(button) || IsMouseButtonHold(button); }
    
    const CursorPositon& GetCursorPosition() const noexcept { return m_currCursorPosition; }
    float GetCursorDx() const noexcept { return m_currCursorPosition.x - m_prevCursorPosition.x; }
    float GetCursorDy() const noexcept { return m_currCursorPosition.y - m_prevCursorPosition.y; }

    bool IsInitialized() const noexcept { return m_isIntialized; }
    
private:
    bool Init(Window* pWindow) noexcept;
    void Destroy() noexcept;

private:
    void OnKeyEvent(KeyboardKey key, KeyState state) noexcept;
    void OnMouseButtonEvent(MouseButton button, MouseButtonState state) noexcept;
    void OnMouseMoveEvent(double xpos, double ypos) noexcept;

private:
    enum InputEventIndex
    {
        IDX_CURSOR_MOVED,
        IDX_CURSOR_LEAVED,
        IDX_CURSOR_ENTERED,
        IDX_MOUSE_PRESSED,
        IDX_MOUSE_RELEASED,
        IDX_MOUSE_HOLD,
        IDX_MOUSE_WHEEL,
        IDX_KEY_PRESSED,
        IDX_KEY_RELEASED,
        IDX_KEY_HOLD,

        IDX_COUNT,
    };

    struct InputEventListenerDesc
    {
        uint64_t id;
        uint64_t typeIndexHash;
    };

    std::array<KeyState, static_cast<size_t>(KeyboardKey::KEY_COUNT)> m_keyStates;
    std::array<InputEventListenerDesc, InputEventIndex::IDX_COUNT> m_inputListenersIDDescs;

    CursorPositon m_prevCursorPosition;
    CursorPositon m_currCursorPosition;
    
    std::array<MouseButtonState, static_cast<size_t>(MouseButton::BUTTON_COUNT)> m_mouseButtonStates;
    bool m_isIntialized = false;
};


struct WindowCreateInfo
{
    const char* pTitle;
    uint32_t width;
    uint32_t height;
    bool enableVSync;
};


class Window
{
    friend class WindowSystem;

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
    Window() = default;
    ~Window();

    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;
    
    Window(Window&& other) = delete;
    Window& operator=(Window&& other) = delete;

    void PollEvents() noexcept;
    void SwapBuffers() noexcept;

    void Show() noexcept;
    void Hide() noexcept;

    Input& GetInput() noexcept { return m_input; }
    void* GetNativeWindow() noexcept { return m_pNativeWindow; }

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

    bool IsInitialized() const noexcept { return m_pNativeWindow != nullptr; }

private:
    bool Init(const WindowCreateInfo& createInfo) noexcept;
    void Destroy() noexcept;

private:
    enum WindowEventIndex
    {
        IDX_RESIZED,
        IDX_MINIMIZED,
        IDX_MAXIMIZED,
        IDX_SIZE_RESTORED,
        IDX_CLOSED,
        IDX_FOCUSED,
        IDX_UNFOCUSED,
        IDX_FRAMEBUFFER_RESIZED,

        IDX_COUNT,
    };

    struct WindowEventListenerDesc
    {
        uint64_t id;
        uint64_t typeIndexHash;
    };

    Input m_input;

    std::array<WindowEventListenerDesc, WindowEventIndex::IDX_COUNT> m_windowEventListenersIDDescs;

    void* m_pNativeWindow = nullptr;

    uint32_t m_windowWidth = 0;
    uint32_t m_windowHeight = 0;

    uint32_t m_framebufferWidth = 0;
    uint32_t m_framebufferHeight = 0;

    std::bitset<STATE_COUNT> m_state = STATE_CLOSED;
};


enum WindowTypeTag : uint32_t
{
    WINDOW_TAG_MAIN,

    WINDOW_TAG_COUNT
};


class WindowSystem
{
    friend bool engInitWindowSystem() noexcept;
    friend void engTerminateWindowSystem() noexcept;
    friend bool engIsWindowSystemInitialized() noexcept;

public:
    static WindowSystem& GetInstance() noexcept;

public:
    WindowSystem(const WindowSystem& other) = delete;
    WindowSystem& operator=(const WindowSystem& other) = delete;
    WindowSystem(WindowSystem&& other) noexcept = delete;
    WindowSystem& operator=(WindowSystem&& other) noexcept = delete;

    ~WindowSystem();

    Window* CreateWindow(WindowTypeTag tag, const WindowCreateInfo& createInfo) noexcept;
    void DestroyWindow(WindowTypeTag tag) noexcept;
    Window* GetWindowByTag(WindowTypeTag tag) noexcept;

    bool IsInitialized() const noexcept { return m_isInitialized; }

private:
    WindowSystem() = default;

    bool Init() noexcept;
    void Terminate() noexcept;

private:
    std::array<Window, WINDOW_TAG_COUNT> m_windowsStorage;
    bool m_isInitialized = false;
};


Window& engGetMainWindow() noexcept;


bool engInitWindowSystem() noexcept;
void engTerminateWindowSystem() noexcept;
bool engIsWindowSystemInitialized() noexcept;
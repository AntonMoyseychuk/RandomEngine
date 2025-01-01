#include "pch.h"

#include "engine/window_system/input.h"
#include "engine/window_system/window.h"

#include "engine/event_system/event_dispatcher.h"

#include "utils/debug/assertion.h"

#include <GLFW/glfw3.h>


#define ENG_CHECK_BINDED_WINDOW_INIT_STATUS(pWindow) ENG_ASSERT_WINDOW(pWindow, "[Input]: Binded window is nullptr")


#define DECLARE_MOUSE_BUTTON_EVENT_BODY(EventName)          \
public:                                                     \
    EventName(int32_t button)                               \
        : m_button(button) {}                               \
                                                            \
    int32_t GetButton() const noexcept { return m_button; } \
                                                            \
private:                                                    \
    int32_t m_button;


#define DECLARE_KEYBOARD_EVENT_BODY(EventName)                  \
public:                                                         \
    EventName(int32_t key, int32_t scancode)                    \
        : m_key(key), m_scancode(scancode) {}                   \
                                                                \
    int32_t GetKey() const noexcept { return m_key; }           \
    int32_t GetScancode() const noexcept { return m_scancode; } \
                                                                \
private:                                                        \
    int32_t m_key;                                              \
    int32_t m_scancode;



class EventMousePressed
{
    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMousePressed)
};


class EventMouseReleased
{
    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMouseReleased)
};


class EventMouseHold
{
    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMouseHold)
};


class EventCursorLeaved
{
public:
    EventCursorLeaved() = default;
};


class EventCursorEntered
{
public:
    EventCursorEntered() = default;
};


class EventKeyPressed
{
    DECLARE_KEYBOARD_EVENT_BODY(EventKeyPressed)
};


class EventKeyReleased
{
    DECLARE_KEYBOARD_EVENT_BODY(EventKeyReleased)
};


class EventKeyHold
{
    DECLARE_KEYBOARD_EVENT_BODY(EventKeyHold)
};


class EventCursorMoved
{
public:
    EventCursorMoved(float x, float y)
        : m_xpos(x), m_ypos(y) {}

    float GetX() const noexcept { return m_xpos; }
    float GetY() const noexcept { return m_ypos; }

private:
    float m_xpos;
    float m_ypos;
};


class EventMouseWheel
{
public:
    EventMouseWheel(float xoffset, float yoffset)
        : m_xoffset(xoffset), m_yoffset(yoffset) {}

    float GetDX() const noexcept { return m_xoffset; }
    float GetDY() const noexcept { return m_yoffset; }

private:
    float m_xoffset;
    float m_yoffset;
};


static KeyboardKey GLFWKeyToCustomKey(int32_t glfwKey) noexcept
{
    switch (glfwKey) {
        case GLFW_KEY_SPACE: return KeyboardKey::KEY_SPACE;
        case GLFW_KEY_APOSTROPHE: return KeyboardKey::KEY_APOSTROPHE;
        case GLFW_KEY_COMMA: return KeyboardKey::KEY_COMMA;
        case GLFW_KEY_MINUS: return KeyboardKey::KEY_MINUS;
        case GLFW_KEY_PERIOD: return KeyboardKey::KEY_PERIOD;
        case GLFW_KEY_SLASH: return KeyboardKey::KEY_SLASH;
        case GLFW_KEY_0: return KeyboardKey::KEY_0;
        case GLFW_KEY_1: return KeyboardKey::KEY_1;
        case GLFW_KEY_2: return KeyboardKey::KEY_2;
        case GLFW_KEY_3: return KeyboardKey::KEY_3;
        case GLFW_KEY_4: return KeyboardKey::KEY_4;
        case GLFW_KEY_5: return KeyboardKey::KEY_5;
        case GLFW_KEY_6: return KeyboardKey::KEY_6;
        case GLFW_KEY_7: return KeyboardKey::KEY_7;
        case GLFW_KEY_8: return KeyboardKey::KEY_8;
        case GLFW_KEY_9: return KeyboardKey::KEY_9;
        case GLFW_KEY_SEMICOLON: return KeyboardKey::KEY_SEMICOLON;
        case GLFW_KEY_EQUAL: return KeyboardKey::KEY_EQUAL;
        case GLFW_KEY_A: return KeyboardKey::KEY_A;
        case GLFW_KEY_B: return KeyboardKey::KEY_B;
        case GLFW_KEY_C: return KeyboardKey::KEY_C;
        case GLFW_KEY_D: return KeyboardKey::KEY_D;
        case GLFW_KEY_E: return KeyboardKey::KEY_E;
        case GLFW_KEY_F: return KeyboardKey::KEY_F;
        case GLFW_KEY_G: return KeyboardKey::KEY_G;
        case GLFW_KEY_H: return KeyboardKey::KEY_H;
        case GLFW_KEY_I: return KeyboardKey::KEY_I;
        case GLFW_KEY_J: return KeyboardKey::KEY_J;
        case GLFW_KEY_K: return KeyboardKey::KEY_K;
        case GLFW_KEY_L: return KeyboardKey::KEY_L;
        case GLFW_KEY_M: return KeyboardKey::KEY_M;
        case GLFW_KEY_N: return KeyboardKey::KEY_N;
        case GLFW_KEY_O: return KeyboardKey::KEY_O;
        case GLFW_KEY_P: return KeyboardKey::KEY_P;
        case GLFW_KEY_Q: return KeyboardKey::KEY_Q;
        case GLFW_KEY_R: return KeyboardKey::KEY_R;
        case GLFW_KEY_S: return KeyboardKey::KEY_S;
        case GLFW_KEY_T: return KeyboardKey::KEY_T;
        case GLFW_KEY_U: return KeyboardKey::KEY_U;
        case GLFW_KEY_V: return KeyboardKey::KEY_V;
        case GLFW_KEY_W: return KeyboardKey::KEY_W;
        case GLFW_KEY_X: return KeyboardKey::KEY_X;
        case GLFW_KEY_Y: return KeyboardKey::KEY_Y;
        case GLFW_KEY_Z: return KeyboardKey::KEY_Z;
        case GLFW_KEY_LEFT_BRACKET: return KeyboardKey::KEY_LEFT_BRACKET;
        case GLFW_KEY_BACKSLASH: return KeyboardKey::KEY_BACKSLASH;
        case GLFW_KEY_RIGHT_BRACKET: return KeyboardKey::KEY_RIGHT_BRACKET;
        case GLFW_KEY_GRAVE_ACCENT: return KeyboardKey::KEY_GRAVE_ACCENT;
        case GLFW_KEY_ESCAPE: return KeyboardKey::KEY_ESCAPE;
        case GLFW_KEY_ENTER: return KeyboardKey::KEY_ENTER;
        case GLFW_KEY_TAB: return KeyboardKey::KEY_TAB;
        case GLFW_KEY_BACKSPACE: return KeyboardKey::KEY_BACKSPACE;
        case GLFW_KEY_INSERT: return KeyboardKey::KEY_INSERT;
        case GLFW_KEY_DELETE: return KeyboardKey::KEY_DELETE;
        case GLFW_KEY_RIGHT: return KeyboardKey::KEY_RIGHT;
        case GLFW_KEY_LEFT: return KeyboardKey::KEY_LEFT;
        case GLFW_KEY_DOWN: return KeyboardKey::KEY_DOWN;
        case GLFW_KEY_UP: return KeyboardKey::KEY_UP;
        case GLFW_KEY_PAGE_UP: return KeyboardKey::KEY_PAGE_UP;
        case GLFW_KEY_PAGE_DOWN: return KeyboardKey::KEY_PAGE_DOWN;
        case GLFW_KEY_HOME: return KeyboardKey::KEY_HOME;
        case GLFW_KEY_END: return KeyboardKey::KEY_END;
        case GLFW_KEY_CAPS_LOCK: return KeyboardKey::KEY_CAPS_LOCK;
        case GLFW_KEY_SCROLL_LOCK: return KeyboardKey::KEY_SCROLL_LOCK;
        case GLFW_KEY_NUM_LOCK: return KeyboardKey::KEY_NUM_LOCK;
        case GLFW_KEY_PRINT_SCREEN: return KeyboardKey::KEY_PRINT_SCREEN;
        case GLFW_KEY_PAUSE: return KeyboardKey::KEY_PAUSE;
        case GLFW_KEY_F1: return KeyboardKey::KEY_F1;
        case GLFW_KEY_F2: return KeyboardKey::KEY_F2;
        case GLFW_KEY_F3: return KeyboardKey::KEY_F3;
        case GLFW_KEY_F4: return KeyboardKey::KEY_F4;
        case GLFW_KEY_F5: return KeyboardKey::KEY_F5;
        case GLFW_KEY_F6: return KeyboardKey::KEY_F6;
        case GLFW_KEY_F7: return KeyboardKey::KEY_F7;
        case GLFW_KEY_F8: return KeyboardKey::KEY_F8;
        case GLFW_KEY_F9: return KeyboardKey::KEY_F9;
        case GLFW_KEY_F10: return KeyboardKey::KEY_F10;
        case GLFW_KEY_F11: return KeyboardKey::KEY_F11;
        case GLFW_KEY_F12: return KeyboardKey::KEY_F12;
        case GLFW_KEY_F13: return KeyboardKey::KEY_F13;
        case GLFW_KEY_F14: return KeyboardKey::KEY_F14;
        case GLFW_KEY_F15: return KeyboardKey::KEY_F15;
        case GLFW_KEY_F16: return KeyboardKey::KEY_F16;
        case GLFW_KEY_F17: return KeyboardKey::KEY_F17;
        case GLFW_KEY_F18: return KeyboardKey::KEY_F18;
        case GLFW_KEY_F19: return KeyboardKey::KEY_F19;
        case GLFW_KEY_F20: return KeyboardKey::KEY_F20;
        case GLFW_KEY_F21: return KeyboardKey::KEY_F21;
        case GLFW_KEY_F22: return KeyboardKey::KEY_F22;
        case GLFW_KEY_F23: return KeyboardKey::KEY_F23;
        case GLFW_KEY_F24: return KeyboardKey::KEY_F24;
        case GLFW_KEY_F25: return KeyboardKey::KEY_F25;
        case GLFW_KEY_KP_0: return KeyboardKey::KEY_KP_0;
        case GLFW_KEY_KP_1: return KeyboardKey::KEY_KP_1;
        case GLFW_KEY_KP_2: return KeyboardKey::KEY_KP_2;
        case GLFW_KEY_KP_3: return KeyboardKey::KEY_KP_3;
        case GLFW_KEY_KP_4: return KeyboardKey::KEY_KP_4;
        case GLFW_KEY_KP_5: return KeyboardKey::KEY_KP_5;
        case GLFW_KEY_KP_6: return KeyboardKey::KEY_KP_6;
        case GLFW_KEY_KP_7: return KeyboardKey::KEY_KP_7;
        case GLFW_KEY_KP_8: return KeyboardKey::KEY_KP_8;
        case GLFW_KEY_KP_9: return KeyboardKey::KEY_KP_9;
        case GLFW_KEY_KP_DECIMAL: return KeyboardKey::KEY_KP_DECIMAL;
        case GLFW_KEY_KP_DIVIDE: return KeyboardKey::KEY_KP_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY: return KeyboardKey::KEY_KP_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT: return KeyboardKey::KEY_KP_SUBTRACT;
        case GLFW_KEY_KP_ADD: return KeyboardKey::KEY_KP_ADD;
        case GLFW_KEY_KP_ENTER: return KeyboardKey::KEY_KP_ENTER;
        case GLFW_KEY_KP_EQUAL: return KeyboardKey::KEY_KP_EQUAL;
        case GLFW_KEY_LEFT_SHIFT: return KeyboardKey::KEY_LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return KeyboardKey::KEY_LEFT_CONTROL;
        case GLFW_KEY_LEFT_ALT: return KeyboardKey::KEY_LEFT_ALT;
        case GLFW_KEY_LEFT_SUPER: return KeyboardKey::KEY_LEFT_SUPER;
        case GLFW_KEY_RIGHT_SHIFT: return KeyboardKey::KEY_RIGHT_SHIFT;
        case GLFW_KEY_RIGHT_CONTROL: return KeyboardKey::KEY_RIGHT_CONTROL;
        case GLFW_KEY_RIGHT_ALT: return KeyboardKey::KEY_RIGHT_ALT;
        case GLFW_KEY_RIGHT_SUPER: return KeyboardKey::KEY_RIGHT_SUPER;
        case GLFW_KEY_MENU: return KeyboardKey::KEY_MENU;
    
        default:
            ENG_ASSERT_WINDOW_FAIL("Invalid GLFW key: {}", glfwKey);
            return KeyboardKey::KEY_COUNT;
    }
}


static MouseButton GLFWButtonToCustomMouseButton(int32_t glfwButton) noexcept
{
    switch (glfwButton) {
        case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::BUTTON_LEFT;
        case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::BUTTON_RIGHT;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::BUTTON_MIDDLE;
    
        default:
            ENG_ASSERT_WINDOW_FAIL("Invalid GLFW mouse button: {}", glfwButton);
            return MouseButton::BUTTON_COUNT;
    }
}


Input::Input(Window* pWindow)
{
    const bool isWindowBinded = BindWindow(pWindow);
    ENG_ASSERT_WINDOW(isWindowBinded, "Window binding failed");
}


KeyState Input::GetKeyState(KeyboardKey key) const noexcept
{
    const size_t keyIndex = static_cast<size_t>(key);
    ENG_ASSERT_WINDOW(keyIndex < static_cast<size_t>(KeyboardKey::KEY_COUNT), "Invalid key index");

    return m_keyStates[keyIndex];
}

MouseButtonState Input::GetMouseButtonState(MouseButton button) const noexcept
{
    const size_t buttonIndex = static_cast<size_t>(button);
    ENG_ASSERT_WINDOW(buttonIndex < static_cast<size_t>(MouseButton::BUTTON_COUNT), "Invalid mouse button index");

    return m_mouseButtonStates[buttonIndex];
}


bool Input::BindWindow(Window* pWindow) noexcept
{
    if (!pWindow) {
        ENG_LOG_WINDOW_WARN("pWindow is nullptr");
        return false;
    }

    if (!pWindow->IsWindowInitialized()) {
        ENG_LOG_WINDOW_WARN("pWindow is not initialized");
        return false;
    }

    m_pBoundWindow = pWindow;

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    dispatcher.Subscribe(
        EventListener::Create<EventKeyPressed>([this](const void* pEvent) 
        {
            OnKeyEvent(GLFWKeyToCustomKey(CastEventTo<EventKeyPressed>(pEvent).GetKey()), KeyState::STATE_PRESSED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventKeyReleased>([this](const void* pEvent)
        {
            OnKeyEvent(GLFWKeyToCustomKey(CastEventTo<EventKeyReleased>(pEvent).GetKey()), KeyState::STATE_RELEASED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventKeyHold>([this](const void* pEvent) 
        {
            OnKeyEvent(GLFWKeyToCustomKey(CastEventTo<EventKeyHold>(pEvent).GetKey()), KeyState::STATE_HOLD);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventMousePressed>([this](const void* pEvent)
        {
            OnMouseButtonEvent(GLFWButtonToCustomMouseButton(CastEventTo<EventMousePressed>(pEvent).GetButton()), MouseButtonState::STATE_PRESSED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventMouseReleased>([this](const void* pEvent)
        {
            OnMouseButtonEvent(GLFWButtonToCustomMouseButton(CastEventTo<EventMouseReleased>(pEvent).GetButton()), MouseButtonState::STATE_RELEASED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventMouseHold>([this](const void* pEvent)
        {
            OnMouseButtonEvent(GLFWButtonToCustomMouseButton(CastEventTo<EventMouseHold>(pEvent).GetButton()), MouseButtonState::STATE_HOLD);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventCursorMoved>([this](const void* pEvent)
        {
            const EventCursorMoved& event = CastEventTo<EventCursorMoved>(pEvent);
            OnMouseMoveEvent(event.GetX(), event.GetY());
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventCursorMoved>([this](const void* pEvent) {

        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventCursorEntered>([this](const void* pEvent) {
        
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventMouseWheel>([this](const void* pEvent) {
        
        }
    ));

    glfwSetKeyCallback(m_pBoundWindow->m_pWindow, [](GLFWwindow* pWindow, int32_t key, int32_t scancode, int32_t action, int32_t mods){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        
        switch (action) {
            case GLFW_PRESS:
                dispatcher.Notify<EventKeyPressed>(key, scancode);
                break;
            case GLFW_RELEASE:
                dispatcher.Notify<EventKeyReleased>(key, scancode);
                break;
            case GLFW_REPEAT:
                dispatcher.Notify<EventKeyHold>(key, scancode);
                break;
            default:
                break;
        }
    });

    glfwSetCursorPosCallback(m_pBoundWindow->m_pWindow, [](GLFWwindow* pWindow, double xpos, double ypos){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventCursorMoved>((float)xpos, (float)ypos);
    });

    glfwSetCursorEnterCallback(m_pBoundWindow->m_pWindow, [](GLFWwindow* pWindow, int32_t entered){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();

        if (entered) {
            dispatcher.Notify<EventCursorEntered>();
        } else {
            dispatcher.Notify<EventCursorLeaved>();
        }
    });
    
    
    glfwSetMouseButtonCallback(m_pBoundWindow->m_pWindow, [](GLFWwindow* pWindow, int32_t button, int32_t action, int32_t mods){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        
        switch (action) {
            case GLFW_PRESS:
                dispatcher.Notify<EventMousePressed>(button);
                break;
            case GLFW_RELEASE:
                dispatcher.Notify<EventMouseReleased>(button);
                break;
            case GLFW_REPEAT:
                dispatcher.Notify<EventMouseHold>(button);
                break;
            default:
                break;
        }
    });

    glfwSetScrollCallback(m_pBoundWindow->m_pWindow, [](GLFWwindow* pWindow, double xoffset, double yoffset){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventMouseWheel>((float)xoffset, (float)yoffset);
    });

    return true;
}


void Input::OnKeyEvent(KeyboardKey key, KeyState state) noexcept
{
    m_keyStates[static_cast<size_t>(key)] = state;
}


void Input::OnMouseButtonEvent(MouseButton button, MouseButtonState state) noexcept
{
    m_mouseButtonStates[static_cast<size_t>(button)] = state;
}


void Input::OnMouseMoveEvent(double xpos, double ypos) noexcept
{
    m_prevCursorPosition = m_currCursorPosition;
    
    m_currCursorPosition.x = static_cast<float>(xpos);
    m_currCursorPosition.y = static_cast<float>(ypos);
}

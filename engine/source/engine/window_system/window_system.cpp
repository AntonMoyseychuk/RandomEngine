#include "pch.h"

#include "engine/window_system/window_system.h"
#include "engine/event_system/event_dispatcher.h"

#include "utils/debug/assertion.h"

#include <GLFW/glfw3.h>


#define ENG_CHECK_WINDOW_INIT_STATUS(pWindow) ENG_ASSERT_WINDOW(pWindow, "Window is not initialized")


static std::unique_ptr<WindowSystem> pWindowSysInst = nullptr;


static const char* WindowTypeTagToStr(WindowTypeTag tag) noexcept
{
    switch(tag) {
        case WINDOW_TAG_MAIN: return "WINDOW_TAG_MAIN";
        default:
            ENG_ASSERT_FAIL("Invalid window type tag value");
            return "UNKNOWN";
    }
}


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


bool Input::Init(Window* pWindow) noexcept
{
    if (!(pWindow && pWindow->IsInitialized())) {
        ENG_LOG_WINDOW_WARN("pWindow is invalid");
        return false;
    }

    GLFWwindow* pNativeWindow = static_cast<GLFWwindow*>(pWindow->GetNativeWindow());
    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    
    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventCursorMoved>(
            [this](const void* pEvent) {
                const EventCursorMoved& event = CastEventTo<EventCursorMoved>(pEvent);
                OnMouseMoveEvent(event.GetX(), event.GetY());
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_CURSOR_MOV");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_CURSOR_MOVED].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_CURSOR_MOVED].typeIndexHash = listenerID.TypeIndexHash();
    }
    
    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventMousePressed>(
            [this](const void* pEvent) {
                OnMouseButtonEvent(GLFWButtonToCustomMouseButton(CastEventTo<EventMousePressed>(pEvent).GetButton()), MouseButtonState::STATE_PRESSED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_MOUSE_PRESS");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_PRESSED].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_PRESSED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventMouseReleased>(
            [this](const void* pEvent) {
                OnMouseButtonEvent(GLFWButtonToCustomMouseButton(CastEventTo<EventMouseReleased>(pEvent).GetButton()), MouseButtonState::STATE_RELEASED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_MOUSE_RELEASE");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_RELEASED].id = listenerID.Value(); 
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_RELEASED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventMouseHold>(
            [this](const void* pEvent) {
                OnMouseButtonEvent(GLFWButtonToCustomMouseButton(CastEventTo<EventMouseHold>(pEvent).GetButton()), MouseButtonState::STATE_HOLD);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_MOUSE_HOLD");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_HOLD].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_HOLD].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventKeyPressed>(
            [this](const void* pEvent) {
                OnKeyEvent(GLFWKeyToCustomKey(CastEventTo<EventKeyPressed>(pEvent).GetKey()), KeyState::STATE_PRESSED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_KEY_PRESS");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_KEY_PRESSED].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_KEY_PRESSED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventKeyReleased>(
            [this](const void* pEvent) {
                OnKeyEvent(GLFWKeyToCustomKey(CastEventTo<EventKeyReleased>(pEvent).GetKey()), KeyState::STATE_RELEASED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_KEY_RELEASE");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_KEY_RELEASED].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_KEY_RELEASED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventKeyHold>(
            [this](const void* pEvent) {
                OnKeyEvent(GLFWKeyToCustomKey(CastEventTo<EventKeyHold>(pEvent).GetKey()), KeyState::STATE_HOLD);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_KEY_HOLD");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_KEY_HOLD].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_KEY_HOLD].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventCursorLeaved>([](const void* pEvent) {});

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_CURSOR_LEAVE");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_CURSOR_LEAVED].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_CURSOR_LEAVED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventCursorEntered>([](const void* pEvent) {});

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_CURSOR_ENTER");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_CURSOR_ENTERED].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_CURSOR_ENTERED].typeIndexHash = listenerID.TypeIndexHash();
    }
    
    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventMouseWheel>([](const void* pEvent) {});

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_MOUSE_WHEEL");

        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_WHEEL].id = listenerID.Value();
        m_inputListenersIDDescs[(size_t)InputEventIndex::IDX_MOUSE_WHEEL].typeIndexHash = listenerID.TypeIndexHash();
    }


    glfwSetKeyCallback(pNativeWindow, [](GLFWwindow* pWindow, int32_t key, int32_t scancode, int32_t action, int32_t mods){
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

    glfwSetCursorPosCallback(pNativeWindow, [](GLFWwindow* pWindow, double xpos, double ypos){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventCursorMoved>((float)xpos, (float)ypos);
    });

    glfwSetCursorEnterCallback(pNativeWindow, [](GLFWwindow* pWindow, int32_t entered){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();

        if (entered) {
            dispatcher.Notify<EventCursorEntered>();
        } else {
            dispatcher.Notify<EventCursorLeaved>();
        }
    });
    
    
    glfwSetMouseButtonCallback(pNativeWindow, [](GLFWwindow* pWindow, int32_t button, int32_t action, int32_t mods){
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

    glfwSetScrollCallback(pNativeWindow, [](GLFWwindow* pWindow, double xoffset, double yoffset){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventMouseWheel>((float)xoffset, (float)yoffset);
    });

    m_isIntialized = true;

    return true;
}


void Input::Destroy() noexcept
{
    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    for (WindowSystemEventListenerDesc& desc : m_inputListenersIDDescs) {
        dispatcher.Unsubscribe(EventListenerID(desc.id, desc.typeIndexHash));
        desc = {};
    }

    m_keyStates = {};
    m_prevCursorPosition = {};
    m_currCursorPosition = {};
    m_mouseButtonStates = {};
    m_isIntialized = false;
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


Window::~Window()
{
    Destroy();
}


bool Window::Init(const WindowCreateInfo& createInfo) noexcept
{
    ENG_ASSERT_WINDOW(engIsWindowSystemInitialized(), "Can't create window since window system is not initialized");
    ENG_ASSERT_WINDOW(createInfo.pTitle != nullptr, "Window title is nullptr");

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    
    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowResized>(
            [this](const void* pEvent) {
                const EventWindowResized& event = CastEventTo<EventWindowResized>(pEvent);
                
                m_windowWidth = event.GetWidth();
                m_windowHeight = event.GetHeight();
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_RESIZED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_RESIZED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_RESIZED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowMinimized>(
            [this](const void* pEvent) {
                m_state.set(STATE_MINIMIZED);
                m_state.reset(STATE_MAXIMIZED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_MINIMIZED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_MINIMIZED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_MINIMIZED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowMaximized>(
            [this](const void* pEvent) {
                m_state.set(STATE_MAXIMIZED);
                m_state.reset(STATE_MINIMIZED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_MAXIMIZED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_MAXIMIZED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_MAXIMIZED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowSizeRestored>(
            [this](const void* pEvent) {
                m_state.reset(STATE_MAXIMIZED);
                m_state.reset(STATE_MINIMIZED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_SIZE_RESTORED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_SIZE_RESTORED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_SIZE_RESTORED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowClosed>(
            [this](const void* pEvent) {
                m_state.set(STATE_CLOSED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_CLOSED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_CLOSED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_CLOSED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowFocused>(
            [this](const void* pEvent) {
                m_state.set(STATE_FOCUSED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_FOCUSED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_FOCUSED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_FOCUSED].typeIndexHash = listenerID.TypeIndexHash();
    }
    
    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventWindowUnfocused>(
            [this](const void* pEvent) {
                m_state.reset(STATE_FOCUSED);
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_SYS_UNFOCUSED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_UNFOCUSED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_UNFOCUSED].typeIndexHash = listenerID.TypeIndexHash();
    }

    {
        const EventListenerID listenerID = dispatcher.Subscribe<EventFramebufferResized>(
            [this](const void* pEvent) {
                const EventFramebufferResized& event = CastEventTo<EventFramebufferResized>(pEvent);
                
                m_framebufferWidth = event.GetWidth();
                m_framebufferHeight = event.GetHeight();
            }
        );

        ENG_ASSERT(listenerID.IsValid(), "Invalid event listener ID");
        dispatcher.SetListenerDebugName(listenerID, "WINDOW_FRAMEBUFFER_RESIZED");

        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_FRAMEBUFFER_RESIZED].id = listenerID.Value();
        m_windowEventListenersIDDescs[(size_t)WindowEventIndex::IDX_FRAMEBUFFER_RESIZED].typeIndexHash = listenerID.TypeIndexHash();
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    m_pNativeWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.pTitle, nullptr, nullptr);
    ENG_ASSERT_WINDOW(m_pNativeWindow, "Window creation failed");
    GLFWwindow* pGLFWWindow = static_cast<GLFWwindow*>(m_pNativeWindow);

    m_windowWidth = createInfo.width;
    m_windowHeight = createInfo.height;

    int32_t framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(pGLFWWindow, &framebufferWidth, &framebufferHeight);
    m_framebufferWidth = framebufferWidth;
    m_framebufferHeight = framebufferHeight;

    glfwMakeContextCurrent(pGLFWWindow);
    glfwSwapInterval(createInfo.enableVSync);

    glfwSetWindowCloseCallback(pGLFWWindow, [](GLFWwindow* pWindow){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventWindowClosed>();
    });

    glfwSetWindowIconifyCallback(pGLFWWindow, [](GLFWwindow* pWindow, int32_t iconified){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        if (iconified) {
            dispatcher.Notify<EventWindowMinimized>();
        } else {
            dispatcher.Notify<EventWindowSizeRestored>();
        }
    });

    glfwSetWindowMaximizeCallback(pGLFWWindow, [](GLFWwindow* pWindow, int32_t maximized){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        if (maximized) {
            dispatcher.Notify<EventWindowMaximized>();
        } else {
            dispatcher.Notify<EventWindowSizeRestored>();
        }
    });

    glfwSetWindowFocusCallback(pGLFWWindow, [](GLFWwindow* pWindow, int32_t focused){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        if (focused) {
            dispatcher.Notify<EventWindowFocused>();
        } else {
            dispatcher.Notify<EventWindowUnfocused>();
        }
    });

    glfwSetWindowSizeCallback(pGLFWWindow, [](GLFWwindow* pWindow, int32_t width, int32_t height){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventWindowResized>(width, height);
    });

    glfwSetFramebufferSizeCallback(pGLFWWindow, [](GLFWwindow* pWindow, int32_t width, int32_t height){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventFramebufferResized>(width, height);
    });

    m_input.Init(this);
    ENG_ASSERT_WINDOW(m_input.IsInitialized(), "Input system initialization failed");

    m_state.reset(STATE_CLOSED);

    return true;
}


void Window::Destroy() noexcept
{
    if (!IsInitialized()) {
        return;
    }
    
    m_input.Destroy();

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    for (WindowSystemEventListenerDesc& desc : m_windowEventListenersIDDescs) {
        dispatcher.Unsubscribe(EventListenerID(desc.id, desc.typeIndexHash));
        desc = {};
    }

    glfwDestroyWindow(static_cast<GLFWwindow*>(m_pNativeWindow));
    m_pNativeWindow = nullptr;

    m_windowWidth = 0;
    m_windowHeight = 0;

    m_framebufferWidth = 0;
    m_framebufferHeight = 0;

    m_state = STATE_CLOSED;
}


void Window::PollEvents() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    glfwPollEvents();
}


void Window::SwapBuffers() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    glfwSwapBuffers(static_cast<GLFWwindow*>(m_pNativeWindow));
}


void Window::Show() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    glfwShowWindow(static_cast<GLFWwindow*>(m_pNativeWindow));
}


void Window::Hide() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    glfwHideWindow(static_cast<GLFWwindow*>(m_pNativeWindow));
}


const char* Window::GetTitle() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    return glfwGetWindowTitle(static_cast<GLFWwindow*>(m_pNativeWindow));
}


void Window::SetTitle(const char *title) noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    glfwSetWindowTitle(static_cast<GLFWwindow*>(m_pNativeWindow), title);
}


bool Window::IsVisible() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pNativeWindow);
    return glfwGetWindowAttrib(static_cast<GLFWwindow*>(m_pNativeWindow), GLFW_VISIBLE);
}


WindowSystem& WindowSystem::GetInstance() noexcept
{
    ENG_ASSERT_WINDOW(engIsWindowSystemInitialized(), "Window system is not initialized");
    return *pWindowSysInst;
}


WindowSystem::~WindowSystem()
{
    Terminate();
}


Window* WindowSystem::CreateWindow(WindowTypeTag tag, const WindowCreateInfo &createInfo) noexcept
{
    ENG_ASSERT_WINDOW(IsInitialized(), "Window system is not initialized");
    ENG_ASSERT_WINDOW(tag < WINDOW_TAG_COUNT, "Invalid window type tag");

    Window& windowSlot = m_windowsStorage[tag];

    if (windowSlot.IsInitialized()) {
        ENG_LOG_WINDOW_WARN("Window with tag {} is already created", WindowTypeTagToStr(tag));
        return &windowSlot;
    }

    return windowSlot.Init(createInfo) ? &windowSlot : nullptr;
}


void WindowSystem::DestroyWindow(WindowTypeTag tag) noexcept
{
    ENG_ASSERT_WINDOW(IsInitialized(), "Window system is not initialized");
    ENG_ASSERT_WINDOW(tag < WINDOW_TAG_COUNT, "Invalid window type tag");

    m_windowsStorage[tag].Destroy();
}


Window* WindowSystem::GetWindowByTag(WindowTypeTag tag) noexcept
{
    ENG_ASSERT_WINDOW(IsInitialized(), "Window system is not initialized");
    ENG_ASSERT_WINDOW(tag < WINDOW_TAG_COUNT, "Invalid window type tag");

    Window& windowSlot = m_windowsStorage[tag];

    return windowSlot.IsInitialized() ? &windowSlot : nullptr;
}


bool WindowSystem::Init() noexcept
{
    if (IsInitialized()) {
        return true;
    }

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glfwSetErrorCallback([](int errorCode, const char* description){
        ENG_ASSERT_WINDOW_FAIL("{} (code: {})", description, errorCode);
    });
#endif

    if (glfwInit() != GLFW_TRUE) {
        ENG_ASSERT_WINDOW_FAIL("Window system lib initialization failed");
        return false;
    }

    m_isInitialized = true;

    return true;
}


void WindowSystem::Terminate() noexcept
{
    for (Window& window : m_windowsStorage) {
        window.Destroy();
    }

    glfwTerminate();
    m_isInitialized = false;
}


bool engInitWindowSystem() noexcept
{
    if (engIsWindowSystemInitialized()) {
        ENG_LOG_WINDOW_WARN("Window system is already initialized!");
        return true;
    }

    pWindowSysInst = std::unique_ptr<WindowSystem>(new WindowSystem);

    if (!pWindowSysInst) {
        ENG_ASSERT_FAIL("Failed to allocate memory for window system");
        return false;
    }

    if (!pWindowSysInst->Init()) {
        ENG_ASSERT_FAIL("Failed to initialized window system");
        return false;
    }

    return true;
}


void engTerminateWindowSystem() noexcept
{
    pWindowSysInst = nullptr;
}


bool engIsWindowSystemInitialized() noexcept
{
    return pWindowSysInst && pWindowSysInst->IsInitialized();
}

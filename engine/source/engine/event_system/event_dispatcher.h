#pragma once

#include <vector>
#include <functional>
#include <optional>

#include <cstdint>


enum class EventType
{
    EVENT_MOUSE_PRESSED,
    EVENT_MOUSE_HOLD,
    EVENT_MOUSE_RELEASED,

    EVENT_CURSOR_MOVED,
    EVENT_CURSOR_LEAVED,
    EVENT_CURSOR_ENTERED,

    EVENT_MOUSE_WHEEL,

    EVENT_KEY_PRESSED,
    EVENT_KEY_HOLD,
    EVENT_KEY_RELEASED,

    EVENT_WINDOW_RESIZED,
    EVENT_WINDOW_MINIMIZED,
    EVENT_WINDOW_MAXIMIZED,
    EVENT_WINDOW_SIZE_RESTORED,
    EVENT_WINDOW_CLOSED,
    EVENT_WINDOW_FOCUSED,
    EVENT_WINDOW_UNFOCUSED,
    EVENT_WINDOW_POS_CHANGED,

    EVENT_FRAMEBUFFER_RESIZED,

    EVENT_COUNT
};


#define DECLARE_MOUSE_BUTTON_EVENT_BODY(EventName)          \
    EventName(int32_t button)                               \
        : m_button(button) {}                               \
                                                            \
    int32_t GetButton() const noexcept { return m_button; } \
                                                            \
private:                                                    \
    int32_t m_button;


#define DECLARE_KEYBOARD_EVENT_BODY(EventName)                  \
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
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_MOUSE_PRESSED; }

    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMousePressed)
};


class EventMouseReleased
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_MOUSE_RELEASED; }

    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMouseReleased)
};


class EventMouseHold
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_MOUSE_HOLD; }

    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMouseHold)
};


class EventCursorMoved
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_CURSOR_MOVED; }

    EventCursorMoved(float x, float y)
        : m_xpos(x), m_ypos(y) {}

    float GetX() const noexcept { return m_xpos; }
    float GetY() const noexcept { return m_ypos; }

private:
    float m_xpos;
    float m_ypos;
};


class EventCursorLeaved
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_CURSOR_LEAVED; }

    EventCursorLeaved() = default;
};


class EventCursorEntered
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_CURSOR_ENTERED; }

    EventCursorEntered() = default;
};


class EventMouseWheel
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_MOUSE_WHEEL; }

    EventMouseWheel(float xoffset, float yoffset)
        : m_xoffset(xoffset), m_yoffset(yoffset) {}

    float GetDX() const noexcept { return m_xoffset; }
    float GetDY() const noexcept { return m_yoffset; }

private:
    float m_xoffset;
    float m_yoffset;
};


class EventKeyPressed
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_KEY_PRESSED; }

    DECLARE_KEYBOARD_EVENT_BODY(EventKeyPressed)
};


class EventKeyReleased
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_KEY_RELEASED; }

    DECLARE_KEYBOARD_EVENT_BODY(EventKeyReleased)
};


class EventKeyHold
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_KEY_HOLD; }

    DECLARE_KEYBOARD_EVENT_BODY(EventKeyHold)
};


class EventWindowResized
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_RESIZED; }

    EventWindowResized(int32_t width, int32_t height)
        : m_width(width), m_height(height) {}

    int32_t GetWidth() const noexcept { return m_width; }
    int32_t GetHeight() const noexcept { return m_height; }

private:
    int32_t m_width;
    int32_t m_height;
};


class EventWindowMinimized
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_MINIMIZED; }
};


class EventWindowMaximized
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_MAXIMIZED; }
};


class EventWindowSizeRestored
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_SIZE_RESTORED; }
};


class EventWindowClosed
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_CLOSED; }
};


class EventWindowFocused
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_FOCUSED; }
};


class EventWindowUnfocused
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_UNFOCUSED; }
};


class EventWindowPositionChanged
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_WINDOW_POS_CHANGED; }

    EventWindowPositionChanged(int32_t x, int32_t y)
        : m_xpos(x), m_ypos(y) {}

    int32_t GetX() const noexcept { return m_xpos; }
    int32_t GetY() const noexcept { return m_ypos; }

private:
    int32_t m_xpos;
    int32_t m_ypos;
};


class EventFramebufferResized
{
public:
    static constexpr EventType GetType() noexcept { return EventType::EVENT_FRAMEBUFFER_RESIZED; }

    EventFramebufferResized(int32_t width, int32_t height)
        : m_width(width), m_height(height) {}

    int32_t GetWidth() const noexcept { return m_width; }
    int32_t GetHeight() const noexcept { return m_height; }

private:
    int32_t m_width;
    int32_t m_height;
};


using MousePressedListener = std::function<void(const EventMousePressed&)>;
using MouseReleasedListener = std::function<void(const EventMouseReleased&)>;
using MouseHoldListener = std::function<void(const EventMouseHold&)>;
using CursorMovedListener = std::function<void(const EventCursorMoved&)>;
using CursorLeavedListener = std::function<void(const EventCursorLeaved&)>;
using CursorEnteredListener = std::function<void(const EventCursorEntered&)>;
using MouseWheelListener = std::function<void(const EventMouseWheel&)>;
using KeyPressedListener = std::function<void(const EventKeyPressed&)>;
using KeyReleasedListener = std::function<void(const EventKeyReleased&)>;
using KeyHoldListener = std::function<void(const EventKeyHold&)>;
using WindowResizedListener = std::function<void(const EventWindowResized&)>;
using WindowMinimizedListener = std::function<void(const EventWindowMinimized&)>;
using WindowMaximizedListener = std::function<void(const EventWindowMaximized&)>;
using WindowSizeResoredListener = std::function<void(const EventWindowSizeRestored&)>;
using WindowClosedListener = std::function<void(const EventWindowClosed&)>;
using WindowFocusedListener = std::function<void(const EventWindowFocused&)>;
using WindowUnfocusedListener = std::function<void(const EventWindowUnfocused&)>;
using WindowPositionListener = std::function<void(const EventWindowPositionChanged&)>;
using FramebufferResizedListener = std::function<void(const EventFramebufferResized&)>;


class EventDispatcher
{
public:
    static EventDispatcher& GetInstance();

public:
    void Subscribe(const MousePressedListener& listener) noexcept;
    void Subscribe(const MouseReleasedListener& listener) noexcept;
    void Subscribe(const MouseHoldListener& listener) noexcept;
    void Subscribe(const CursorMovedListener& listener) noexcept;
    void Subscribe(const CursorLeavedListener& listener) noexcept;
    void Subscribe(const CursorEnteredListener& listener) noexcept;
    void Subscribe(const MouseWheelListener& listener) noexcept;
    void Subscribe(const KeyPressedListener& listener) noexcept;
    void Subscribe(const KeyReleasedListener& listener) noexcept;
    void Subscribe(const KeyHoldListener& listener) noexcept;
    void Subscribe(const WindowResizedListener& listener) noexcept;
    void Subscribe(const WindowMinimizedListener& listener) noexcept;
    void Subscribe(const WindowMaximizedListener& listener) noexcept;
    void Subscribe(const WindowSizeResoredListener& listener) noexcept;
    void Subscribe(const WindowClosedListener& listener) noexcept;
    void Subscribe(const WindowFocusedListener& listener) noexcept;
    void Subscribe(const WindowUnfocusedListener& listener) noexcept;
    void Subscribe(const WindowPositionListener& listener) noexcept;
    void Subscribe(const FramebufferResizedListener& listener) noexcept;

    void PushEvent(const EventMousePressed& event) noexcept;
    void PushEvent(const EventMouseReleased& event) noexcept;
    void PushEvent(const EventMouseHold& event) noexcept;
    void PushEvent(const EventCursorMoved& event) noexcept;
    void PushEvent(const EventCursorLeaved& event) noexcept;
    void PushEvent(const EventCursorEntered& event) noexcept;
    void PushEvent(const EventMouseWheel& event) noexcept;
    void PushEvent(const EventKeyPressed& event) noexcept;
    void PushEvent(const EventKeyReleased& event) noexcept;
    void PushEvent(const EventKeyHold& event) noexcept;
    void PushEvent(const EventWindowResized& event) noexcept;
    void PushEvent(const EventWindowMinimized& event) noexcept;
    void PushEvent(const EventWindowMaximized& event) noexcept;
    void PushEvent(const EventWindowSizeRestored& event) noexcept;
    void PushEvent(const EventWindowClosed& event) noexcept;
    void PushEvent(const EventWindowFocused& event) noexcept;
    void PushEvent(const EventWindowUnfocused& event) noexcept;
    void PushEvent(const EventWindowPositionChanged& event) noexcept;
    void PushEvent(const EventFramebufferResized& event) noexcept;

    void ProcessEvents() noexcept;

    void ClearEventQueue(EventType type) noexcept;

private:
    EventDispatcher();

    template<typename EventT, typename EventListenerT>
    void ProcessEvents(const std::vector<EventT>& eventQueue, const std::vector<EventListenerT>& listeners)
    {
        for (const EventT& event : eventQueue) {
            ProcessListeners(event, listeners);
        }
    }

    template<typename EventT, typename EventListenerT>
    void ProcessListeners(const EventT& event, const std::vector<EventListenerT>& listeners)
    {
        for (const EventListenerT& listener : listeners) {
            listener(event);
        }
    }

    void ClearEventQueues() noexcept;

private:
    std::vector<MousePressedListener> m_mousePressedListeners;
    std::vector<MouseReleasedListener> m_mouseReleasedListeners;
    std::vector<MouseHoldListener> m_mouseHoldListeners;
    std::vector<CursorMovedListener> m_cursorMovedListeners;
    std::vector<CursorLeavedListener> m_cursorLeavedListeners;
    std::vector<CursorEnteredListener> m_cursorEnteredListeners;
    std::vector<MouseWheelListener> m_wheelListeners;
    std::vector<KeyPressedListener> m_keyPressedListeners;
    std::vector<KeyReleasedListener> m_keyReleasedListeners;
    std::vector<KeyHoldListener> m_keyHoldListeners;
    std::vector<WindowResizedListener> m_windowResizedListeners;
    std::vector<WindowMinimizedListener> m_windowMinimizedListeners;
    std::vector<WindowMaximizedListener> m_windowMaximizedListeners;
    std::vector<WindowSizeResoredListener> m_windowSizeRestoredListeners;
    std::vector<WindowClosedListener> m_windowClosedListeners;
    std::vector<WindowFocusedListener> m_windowFocusedListeners;
    std::vector<WindowUnfocusedListener> m_windowUnfocusedListeners;
    std::vector<WindowPositionListener> m_windowPositionListeners;
    std::vector<FramebufferResizedListener> m_framebufferResizedListeners;

    std::vector<EventMousePressed> m_mousePressedEventQueue;
    std::vector<EventMouseReleased> m_mouseReleasedEventQueue;
    std::vector<EventMouseHold> m_mouseHoldEventQueue;
    std::vector<EventMouseWheel> m_wheelEventQueue;
    std::vector<EventKeyPressed> m_keyPressedEventQueue;
    std::vector<EventKeyReleased> m_keyReleasedEventQueue;
    std::vector<EventKeyHold> m_keyHoldEventQueue;

    std::optional<EventCursorMoved> m_cursorMovedEvent;
    std::optional<EventCursorLeaved> m_cursorLeavedEvent;
    std::optional<EventCursorEntered> m_cursorEnteredEvent;
    std::optional<EventWindowResized> m_windowResizedEvent;
    std::optional<EventWindowMinimized> m_windowMinimizedEvent;
    std::optional<EventWindowMaximized> m_windowMaximizedEvent;
    std::optional<EventWindowSizeRestored> m_windowSizeRestoredEvent;
    std::optional<EventWindowClosed> m_windowClosedEvent;
    std::optional<EventWindowFocused> m_windowFocusedEvent;
    std::optional<EventWindowUnfocused> m_windowUnfocusedEvent;
    std::optional<EventWindowPositionChanged> m_windowPositionChangedEvent;
    std::optional<EventFramebufferResized> m_framebufferResizedEvent;
};
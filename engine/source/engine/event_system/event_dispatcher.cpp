#include "pch.h"
#include "event_dispatcher.h"

#include "utils/debug/assertion.h"


EventDispatcher& EventDispatcher::GetInstance()
{
    static EventDispatcher dispatcher;
    return dispatcher;
}


void EventDispatcher::Subscribe(const MousePressedListener& listener) noexcept
{
    m_mousePressedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const MouseReleasedListener& listener) noexcept
{
    m_mouseReleasedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const MouseHoldListener& listener) noexcept
{
    m_mouseHoldListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const CursorMovedListener& listener) noexcept
{
    m_cursorMovedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const CursorLeavedListener& listener) noexcept
{
    m_cursorLeavedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const CursorEnteredListener& listener) noexcept
{
    m_cursorEnteredListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const MouseWheelListener& listener) noexcept
{
    m_wheelListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const KeyPressedListener& listener) noexcept
{
    m_keyPressedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const KeyReleasedListener& listener) noexcept
{
    m_keyReleasedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const KeyHoldListener& listener) noexcept
{
    m_keyHoldListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowResizedListener& listener) noexcept
{
    m_windowResizedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowMinimizedListener& listener) noexcept
{
    m_windowMinimizedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowMaximizedListener& listener) noexcept
{
    m_windowMaximizedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowSizeResoredListener & listener) noexcept
{
    m_windowSizeRestoredListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowClosedListener& listener) noexcept
{
    m_windowClosedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowFocusedListener& listener) noexcept
{
    m_windowFocusedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowUnfocusedListener& listener) noexcept
{
    m_windowUnfocusedListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const WindowPositionListener &listener) noexcept
{
    m_windowPositionListeners.emplace_back(listener);
}


void EventDispatcher::Subscribe(const FramebufferResizedListener& listener) noexcept
{
    m_framebufferResizedListeners.emplace_back(listener);
}


void EventDispatcher::PushEvent(const EventMousePressed& event) noexcept
{
    m_mousePressedEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventMouseReleased& event) noexcept
{
    m_mouseReleasedEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventMouseHold& event) noexcept
{
    m_mouseHoldEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventCursorMoved& event) noexcept
{
    m_cursorMovedEvent = event;
}


void EventDispatcher::PushEvent(const EventCursorLeaved& event) noexcept
{
    m_cursorLeavedEvent = event;
}


void EventDispatcher::PushEvent(const EventCursorEntered& event) noexcept
{
    m_cursorEnteredEvent = event;
}


void EventDispatcher::PushEvent(const EventMouseWheel& event) noexcept
{
    m_wheelEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventKeyPressed& event) noexcept
{
    m_keyPressedEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventKeyReleased& event) noexcept
{
    m_keyReleasedEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventKeyHold& event) noexcept
{
    m_keyHoldEventQueue.emplace_back(event);
}


void EventDispatcher::PushEvent(const EventWindowResized& event) noexcept
{
    m_windowResizedEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowMinimized& event) noexcept
{
    m_windowMinimizedEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowMaximized& event) noexcept
{
    m_windowMaximizedEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowSizeRestored &event) noexcept
{
    m_windowSizeRestoredEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowClosed& event) noexcept
{
    m_windowClosedEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowFocused& event) noexcept
{
    m_windowFocusedEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowUnfocused& event) noexcept
{
    m_windowUnfocusedEvent = event;
}


void EventDispatcher::PushEvent(const EventWindowPositionChanged &event) noexcept
{
    m_windowPositionChangedEvent = event;
}


void EventDispatcher::PushEvent(const EventFramebufferResized& event) noexcept
{
    m_framebufferResizedEvent = event;
}


void EventDispatcher::ClearEventQueue(EventType type) noexcept
{
    switch(type) {
        case EventType::EVENT_MOUSE_PRESSED:
            m_mousePressedEventQueue.clear();
            break;
        case EventType::EVENT_MOUSE_RELEASED:
            m_mouseReleasedEventQueue.clear();
            break;
        case EventType::EVENT_MOUSE_HOLD:
            m_mouseHoldEventQueue.clear();
            break;
        case EventType::EVENT_MOUSE_WHEEL:
            m_wheelEventQueue.clear();
            break;
        case EventType::EVENT_KEY_PRESSED:
            m_keyPressedEventQueue.clear();
            break;
        case EventType::EVENT_KEY_RELEASED:
            m_keyReleasedEventQueue.clear();
            break;
        case EventType::EVENT_KEY_HOLD:
            m_keyHoldEventQueue.clear();
            break;
        case EventType::EVENT_CURSOR_MOVED:
            m_cursorMovedEvent.reset();
            break;
        case EventType::EVENT_CURSOR_LEAVED:
            m_cursorLeavedEvent.reset();
            break;
        case EventType::EVENT_CURSOR_ENTERED:
            m_cursorEnteredEvent.reset();
            break;
        case EventType::EVENT_WINDOW_RESIZED:
            m_windowResizedEvent.reset();
            break;
        case EventType::EVENT_WINDOW_MINIMIZED:
            m_windowMinimizedEvent.reset();
            break;
        case EventType::EVENT_WINDOW_MAXIMIZED:
            m_windowMaximizedEvent.reset();
            break;
        case EventType::EVENT_WINDOW_SIZE_RESTORED:
            m_windowSizeRestoredEvent.reset();
            break;
        case EventType::EVENT_WINDOW_CLOSED:
            // end of application;
            break;
        case EventType::EVENT_WINDOW_FOCUSED:
            m_windowFocusedEvent.reset();
            break;
        case EventType::EVENT_WINDOW_UNFOCUSED:
            m_windowUnfocusedEvent.reset();
            break;
        case EventType::EVENT_WINDOW_POS_CHANGED:
            m_windowPositionChangedEvent.reset();
            break;
        case EventType::EVENT_FRAMEBUFFER_RESIZED:
            m_framebufferResizedEvent.reset();
            break;
        default:
            ENG_ASSERT_FAIL("Invalid event type");
            break;
    }
}


void EventDispatcher::ProcessEvents() noexcept
{
    if (m_windowClosedEvent.has_value()) {
        ProcessListeners(m_windowClosedEvent.value(), m_windowClosedListeners);
    }

    if (m_windowMinimizedEvent.has_value()) {
        ProcessListeners(m_windowMinimizedEvent.value(), m_windowMinimizedListeners);
    }

    if (m_windowMaximizedEvent.has_value()) {
        ProcessListeners(m_windowMaximizedEvent.value(), m_windowMaximizedListeners);
    }

    if (m_windowSizeRestoredEvent.has_value()) {
        ProcessListeners(m_windowSizeRestoredEvent.value(), m_windowSizeRestoredListeners);
    }

    if (m_framebufferResizedEvent.has_value()) {
        ProcessListeners(m_framebufferResizedEvent.value(), m_framebufferResizedListeners);
    }

    if (m_cursorMovedEvent.has_value()) {
        ProcessListeners(m_cursorMovedEvent.value(), m_cursorMovedListeners);
    }

    if (m_cursorLeavedEvent.has_value()) {
        ProcessListeners(m_cursorLeavedEvent.value(), m_cursorLeavedListeners);
    }

    if (m_cursorEnteredEvent.has_value()) {
        ProcessListeners(m_cursorEnteredEvent.value(), m_cursorEnteredListeners);
    }

    if (m_windowResizedEvent.has_value()) {
        ProcessListeners(m_windowResizedEvent.value(), m_windowResizedListeners);
    }

    if (m_windowFocusedEvent.has_value()) {
        ProcessListeners(m_windowFocusedEvent.value(), m_windowFocusedListeners);
    }

    if (m_windowUnfocusedEvent.has_value()) {
        ProcessListeners(m_windowUnfocusedEvent.value(), m_windowUnfocusedListeners);
    }

    if (m_windowPositionChangedEvent.has_value()) {
        ProcessListeners(m_windowPositionChangedEvent.value(), m_windowPositionListeners);
    }

    ProcessEvents(m_mousePressedEventQueue, m_mousePressedListeners);
    ProcessEvents(m_mouseHoldEventQueue, m_mouseHoldListeners);
    ProcessEvents(m_mouseReleasedEventQueue, m_mouseReleasedListeners);
    ProcessEvents(m_wheelEventQueue, m_wheelListeners);
    ProcessEvents(m_keyPressedEventQueue, m_keyPressedListeners);
    ProcessEvents(m_keyHoldEventQueue, m_keyHoldListeners);
    ProcessEvents(m_keyReleasedEventQueue, m_keyReleasedListeners);

    ClearEventQueues();
}


EventDispatcher::EventDispatcher()
{
    static constexpr size_t LISTENERS_RESERVE_SIZE = 16ull;
    static constexpr size_t EVENT_QUEUE_RESERVE_SIZE = 64ull;

    m_mousePressedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_mouseReleasedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_mouseHoldListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_cursorMovedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_cursorLeavedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_cursorEnteredListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_wheelListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_keyPressedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_keyReleasedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_keyHoldListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_windowResizedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_windowMinimizedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_windowMaximizedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_windowClosedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_windowFocusedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_windowUnfocusedListeners.reserve(LISTENERS_RESERVE_SIZE);
    m_framebufferResizedListeners.reserve(LISTENERS_RESERVE_SIZE);

    m_mousePressedEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
    m_mouseReleasedEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
    m_mouseHoldEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
    m_wheelEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
    m_keyPressedEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
    m_keyReleasedEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
    m_keyHoldEventQueue.reserve(EVENT_QUEUE_RESERVE_SIZE);
}


void EventDispatcher::ClearEventQueues() noexcept
{
    for (uint32_t queueIdx = 0; queueIdx < static_cast<uint32_t>(EventType::EVENT_COUNT); ++queueIdx) {
        ClearEventQueue(static_cast<EventType>(queueIdx));
    }
}
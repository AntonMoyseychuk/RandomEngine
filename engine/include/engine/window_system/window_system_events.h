#pragma once

#include <cstdint>


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


#define DECALRE_EMPTY_EVENT(EventName) struct EventName {}


struct EventMousePressed
{
    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMousePressed)
};


struct EventMouseReleased
{
    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMouseReleased)
};


struct EventMouseHold
{
    DECLARE_MOUSE_BUTTON_EVENT_BODY(EventMouseHold)
};


struct EventKeyPressed
{
    DECLARE_KEYBOARD_EVENT_BODY(EventKeyPressed)
};


struct EventKeyReleased
{
    DECLARE_KEYBOARD_EVENT_BODY(EventKeyReleased)
};


struct EventKeyHold
{
    DECLARE_KEYBOARD_EVENT_BODY(EventKeyHold)
};


struct EventCursorMoved
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


struct EventMouseWheel
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


DECALRE_EMPTY_EVENT(EventCursorLeaved);
DECALRE_EMPTY_EVENT(EventCursorEntered);


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


DECALRE_EMPTY_EVENT(EventWindowMinimized);
DECALRE_EMPTY_EVENT(EventWindowMaximized);
DECALRE_EMPTY_EVENT(EventWindowSizeRestored);
DECALRE_EMPTY_EVENT(EventWindowClosed);
DECALRE_EMPTY_EVENT(EventWindowFocused);
DECALRE_EMPTY_EVENT(EventWindowUnfocused);
#include "pch.h"
#include "engine/window_system/window.h"

#include "engine/event_system/event_dispatcher.h"

#include "utils/debug/assertion.h"

#include <GLFW/glfw3.h>


#define ENG_CHECK_WINDOW_INIT_STATUS(pWindow) ENG_ASSERT_WINDOW(pWindow, "Window is not initialized")


static bool g_isWindowSystemInitialized = false;


Window::Window(const char* title, uint32_t width, uint32_t height)
{
    if (!g_isWindowSystemInitialized) {
        ENG_ASSERT_WINDOW_FAIL("Can't create window since window system is not initialized");
        return;
    }

    if (!title) {
        ENG_ASSERT_WINDOW_FAIL("Window title is nullptr");
        return;
    }

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
    
    dispatcher.Subscribe(
        EventListener::Create<EventWindowClosed>([this](const void* pEvent) {
            m_state.set(STATE_CLOSED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventWindowFocused>([this](const void* pEvent) {
            m_state.set(STATE_FOCUSED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventWindowUnfocused>([this](const void* pEvent) {
            m_state.reset(STATE_FOCUSED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventWindowMaximized>([this](const void* pEvent) {
            m_state.set(STATE_MAXIMIZED);
            m_state.reset(STATE_MINIMIZED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventWindowMinimized>([this](const void* pEvent) {
            m_state.set(STATE_MINIMIZED);
            m_state.reset(STATE_MAXIMIZED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventWindowSizeRestored>([this](const void* pEvent) {
            m_state.reset(STATE_MAXIMIZED);
            m_state.reset(STATE_MINIMIZED);
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventWindowResized>([this](const void* pEvent) {
            const EventWindowResized& event = CastEventTo<EventWindowResized>(pEvent);
            
            m_windowWidth = event.GetWidth();
            m_windowHeight = event.GetHeight();
        }
    ));

    dispatcher.Subscribe(
        EventListener::Create<EventFramebufferResized>([this](const void* pEvent) {
            const EventFramebufferResized& event = CastEventTo<EventFramebufferResized>(pEvent);
            
            m_framebufferWidth = event.GetWidth();
            m_framebufferHeight = event.GetHeight();
        }
    ));


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    ENG_ASSERT_WINDOW(m_pWindow, "Window creation failed");

    m_windowWidth = width;
    m_windowHeight = height;

    int32_t framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_pWindow, &framebufferWidth, &framebufferHeight);
    m_framebufferWidth = framebufferWidth;
    m_framebufferHeight = framebufferHeight;

    glfwSetWindowUserPointer(m_pWindow, this);

    glfwMakeContextCurrent(m_pWindow);
    glfwSwapInterval(0);

    glfwSetWindowCloseCallback(m_pWindow, [](GLFWwindow* pWindow){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventWindowClosed>();
    });

    glfwSetWindowIconifyCallback(m_pWindow, [](GLFWwindow* pWindow, int32_t iconified){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        if (iconified) {
            dispatcher.Notify<EventWindowMinimized>();
        } else {
            dispatcher.Notify<EventWindowSizeRestored>();
        }
    });

    glfwSetWindowMaximizeCallback(m_pWindow, [](GLFWwindow* pWindow, int32_t maximized){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        if (maximized) {
            dispatcher.Notify<EventWindowMaximized>();
        } else {
            dispatcher.Notify<EventWindowSizeRestored>();
        }
    });

    glfwSetWindowFocusCallback(m_pWindow, [](GLFWwindow* pWindow, int32_t focused){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        if (focused) {
            dispatcher.Notify<EventWindowFocused>();
        } else {
            dispatcher.Notify<EventWindowUnfocused>();
        }
    });

    glfwSetWindowSizeCallback(m_pWindow, [](GLFWwindow* pWindow, int32_t width, int32_t height){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventWindowResized>(width, height);
    });

    glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* pWindow, int32_t width, int32_t height){
        static EventDispatcher& dispatcher = EventDispatcher::GetInstance();
        dispatcher.Notify<EventFramebufferResized>(width, height);
    });

    m_input.BindWindow(this);
    ENG_ASSERT_WINDOW(m_input.IsInitialized(), "Input system initialization failed");

    m_state.reset(STATE_CLOSED);
}


Window::~Window()
{
    glfwDestroyWindow(m_pWindow);
}


void Window::PollEvents() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    glfwPollEvents();
}


void Window::SwapBuffers() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    glfwSwapBuffers(m_pWindow);
}


void Window::ShowWindow() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    glfwShowWindow(m_pWindow);
}


void Window::HideWindow() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    glfwHideWindow(m_pWindow);
}


const char* Window::GetTitle() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    return glfwGetWindowTitle(m_pWindow);
}


void Window::SetTitle(const char *title) noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    glfwSetWindowTitle(m_pWindow, title);
}


bool Window::IsVisible() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    return glfwGetWindowAttrib(m_pWindow, GLFW_VISIBLE);
}


bool engInitWindowSystem() noexcept
{
    if (g_isWindowSystemInitialized) {
        ENG_LOG_WINDOW_WARN("Window system is already initialized!");
        return true;
    }

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glfwSetErrorCallback([](int errorCode, const char* description){
        ENG_ASSERT_WINDOW_FAIL("{} (code: {})", description, errorCode);
    });
#endif

    g_isWindowSystemInitialized = glfwInit() == GLFW_TRUE;

    if (!g_isWindowSystemInitialized) {
        ENG_ASSERT_WINDOW_FAIL("Window system initialization failed");
        return false;
    }

    return true;
}


void engTerminateWindowSystem() noexcept
{
    glfwTerminate();
    g_isWindowSystemInitialized = false;
}


bool engIsWindowSystemInitialized() noexcept
{
    return g_isWindowSystemInitialized;
}
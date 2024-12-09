#include "pch.h"
#include "engine/window_system/window.h"

#include "utils/debug/assertion.h"

#include <GLFW/glfw3.h>


#define ENG_CHECK_WINDOW_INIT_STATUS(pWindow) ENG_ASSERT_WINDOW(pWindow, "Window is not initialized")


static bool g_isWindowSystemInitialized = false;


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

    return g_isWindowSystemInitialized;
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    ENG_ASSERT_WINDOW(m_pWindow, "Window creation failed");

    // glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetWindowUserPointer(m_pWindow, this);

    glfwMakeContextCurrent(m_pWindow);
    glfwSwapInterval(1);

    m_input.BindWindow(this);
    ENG_ASSERT_WINDOW(m_input.IsInitialized(), "Input system initialization failed");

    // glfwSetFramebufferSizeCallback(m_pWindow, nullptr);
}


void Window::ProcessEvents() noexcept
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


void Window::Close() noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE);
}


uint32_t Window::GetWidth() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    
    int32_t width = 0;
    glfwGetFramebufferSize(m_pWindow, &width, nullptr);

    return width;
}


uint32_t Window::GetHeight() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);

    int32_t height = 0;
    glfwGetFramebufferSize(m_pWindow, nullptr, &height);

    return height;
}


bool Window::IsClosed() const noexcept
{
    ENG_CHECK_WINDOW_INIT_STATUS(m_pWindow);
    return glfwWindowShouldClose(m_pWindow);
}
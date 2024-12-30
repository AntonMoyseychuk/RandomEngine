#include "pch.h"
#include "opengl_driver.h"

#include <GLFW/glfw3.h>

#include "utils/debug/assertion.h"


static bool g_isInitialized = false;


bool engIsOpenGLDriverInitialized() noexcept
{
    return g_isInitialized;
}


bool engInitOpenGLDriver() noexcept
{
    if (engIsOpenGLDriverInitialized()) {
        ENG_LOG_GRAPHICS_API_WARN("OpenGL driver is already initialized!");
        return true;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ENG_ASSERT_GRAPHICS_API_FAIL("Failed to initialize OpenGL driver");
        return false;
    }

    g_isInitialized = true;

    return true;
}
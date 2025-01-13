#include "pch.h"
#include "opengl_driver.h"

#include <GLFW/glfw3.h>

#include "utils/debug/assertion.h"


static bool g_isInitialized = false;


#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
static void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* pMessage, const void* userParam)
{
    const char* pSourceStr = [source]() -> const char* {
		switch (source) {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER: return "OTHER";
            default: return "UNDEFINED SOURCE";
		}
	}();

	const char* pTypeStr = [type]() -> const char* {
		switch (type) {
            case GL_DEBUG_TYPE_ERROR: return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER: return "MARKER";
            case GL_DEBUG_TYPE_OTHER: return "OTHER";
            default: return "UNDEFINED TYPE";
		}
	}();

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        ENG_ASSERT_GRAPHICS_API_FAIL("[{}] ({}): {}", pSourceStr, pTypeStr, pMessage);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        ENG_LOG_GRAPHICS_API_WARN("[{}] ({}): {}", pSourceStr, pTypeStr, pMessage);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        ENG_LOG_GRAPHICS_API_INFO("[{}] ({}): {}", pSourceStr, pTypeStr, pMessage);
        break;
    }
}
#endif


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

#if defined(ENG_DEBUG) && defined(ENG_LOGGING_ENABLED)
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
#endif

    g_isInitialized = true;

    return true;
}
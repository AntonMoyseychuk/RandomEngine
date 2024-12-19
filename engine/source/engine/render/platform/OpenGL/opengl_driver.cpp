#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils/debug/assertion.h"


#ifdef OPENGL_DRIVER_H
    #error do NOT include "opengl_driver.h" here!
#endif


struct OpenGLBindableState
{
    GLuint boundShaderProgram = 0;
    GLuint boundVertexArray = 0;
};


static OpenGLBindableState g_OpenGLBindableState;

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


void drvBindShaderProgram(GLuint programID) noexcept
{
    if (g_OpenGLBindableState.boundShaderProgram != programID) {
        glUseProgram(programID);
        g_OpenGLBindableState.boundShaderProgram = programID;
    }
}


void drvBindVertexArray(GLuint arrayID) noexcept
{
    if (g_OpenGLBindableState.boundVertexArray != arrayID) {
        glBindVertexArray(arrayID);
        g_OpenGLBindableState.boundVertexArray = arrayID;
    }
}
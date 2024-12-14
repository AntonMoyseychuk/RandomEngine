#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#ifdef OPENGL_DRIVER_H
    #error do NOT include "opengl_driver.h" here
#endif


struct OpenGLBindableState
{
    GLuint boundShaderProgram = 0;
    GLuint boundVertexArray = 0;
};


static OpenGLBindableState g_OpenGLBindableState;

static bool g_isInitialized = false;


bool engInitOpenGLDriver() noexcept
{
    if (g_isInitialized) {
        return true;
    }

    g_isInitialized = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    return g_isInitialized;
}


bool engIsOpenGLDriverInitialized() noexcept
{
    return g_isInitialized;
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
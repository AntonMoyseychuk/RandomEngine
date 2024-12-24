#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils/debug/assertion.h"

#include <vector>


#ifdef OPENGL_DRIVER_H
    #error do NOT include "opengl_driver.h" here!
#endif


using OpenGLTextureID = GLuint;


struct OpenGLBindableState
{
    std::vector<OpenGLTextureID> textureUnits;

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

    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

    g_OpenGLBindableState.textureUnits.resize(maxTextureUnits);

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


void drvBindTextureUnit(GLuint unit, GLuint textureID) noexcept
{
    std::vector<OpenGLTextureID>& textureUnits = g_OpenGLBindableState.textureUnits;

    ENG_ASSERT_GRAPHICS_API(unit < textureUnits.size(), "texture unit ({}) is greater than max available texture units count ({})", unit, textureUnits.size());

    if (textureUnits[unit] != textureID) {
        glBindTextureUnit(unit, textureID);
        textureUnits[unit] = textureID;
    }
}


void drvUnbindTextureUnit(GLuint unit, GLuint textureID) noexcept
{
    std::vector<OpenGLTextureID>& textureUnits = g_OpenGLBindableState.textureUnits;

    ENG_ASSERT_GRAPHICS_API(unit < textureUnits.size(), "texture unit ({}) is greater than max available texture units count ({})", unit, textureUnits.size());
    
    const bool isUnitBound = textureUnits[unit] != 0;
    if (isUnitBound) {
        ENG_ASSERT_GRAPHICS_API(textureUnits[unit] == textureID, "attempt to unbind texture (id: {}) from unit ({}) by texture with another id ({})", textureUnits[unit], unit, textureID);

        glBindTextureUnit(unit, 0);
        textureUnits[unit] = 0;
    }
}
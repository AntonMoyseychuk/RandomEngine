#include "pch.h"
#include "opengl_driver.h"

#include <GLFW/glfw3.h>

#include "utils/debug/assertion.h"


struct OpenGLGlobalInfo
{
    int32_t majorVersion;
    int32_t minorVersion;
    int32_t maxComputeShaderStorageBlocksCount;
    int32_t maxCombinedShaderStorageBlocksCount;
    int32_t maxComputeUniformBlocksCount;
    int32_t maxComputeTextureImageUnitsCount;
    int32_t maxComputeUniformComponentsCount;
    int32_t maxComputeAtomicCountersCount;
    int32_t maxComputeAtomicCounterBuffersCount;
    int32_t maxCombinedComputeUniformComponentsCount;
    int32_t maxComputeWorkGroupInvocationsCount;
    int32_t maxComputeWorkGroupCountX;
    int32_t maxComputeWorkGroupCountY;
    int32_t maxComputeWorkGroupCountZ;
    int32_t maxComputeWorkGroupSizeX;
    int32_t maxComputeWorkGroupSizeY;
    int32_t maxComputeWorkGroupSizeZ;
    int32_t maxDebugGroupStackDepth;
    int32_t max3DTextureSize;
    int32_t maxArrayTextureLayersCount;
    int32_t maxClipDistancesCount;
    int32_t maxColorTextureSamplersCount;
    int32_t maxCombinedAtomicCountersCount;
    int32_t maxCombinedFragmentUniformComponentsCount;
    int32_t maxCombinedGeometryUniformComponentsCount;
    int32_t maxCombinedTextureImageUnitsCount;
    int32_t maxCombinedUniformBlocksCount;
    int32_t maxCombinedVertexUniformComponentsCount;
    int32_t maxCubeMapTextureSize;
    int32_t maxDepthTextureSamplesCount;
    int32_t maxDrawBuffersCount;
    int32_t maxDualSourceDrawBuffersCount;
    int32_t maxElementIndicesCount;
    int32_t maxElementVerticesCount;
    int32_t maxFragmentAtomicCountersCount;
    int32_t maxFragmentShaderStorageBlocksCount;
    int32_t maxFragmentInputComponentsCount;
    int32_t maxFragmentUniformComponentsCount;
    int32_t maxFragmentUniformVectorsCount;
    int32_t maxFragmentUniformBlocksCount;
    int32_t maxFrameBufferWidth;
    int32_t maxFrameBufferHeight;
    int32_t maxFrameBufferLayersCount;
    int32_t maxFrameSamplesCount;
    int32_t maxGeometryAtomicCountersCount;
    int32_t maxGeometryShaderStorageBlocksCount;
    int32_t maxGeometryInputComponentsCount;
    int32_t maxGeometryOutputComponentsCount;
    int32_t maxGeometryTextureImageUnitsCount;
    int32_t maxGeometryUniformBlocksCount;
    int32_t maxGeometryUniformComponentsCount;
    int32_t maxIntegerSamplesCount;
    int32_t minMapBufferAlignment;
    int32_t maxRectangleTextureSize;
    int32_t maxRenderBufferSize;
    int32_t maxSampleMaskWordsCount;
    int32_t maxServerWaitTimeout;
    int32_t maxShaderStorageBufferBindingsCount;
    int32_t maxTesConrolAtomicCountersCount;
    int32_t maxTesEvaluationAtomicCountersCount;
    int32_t maxTesControlShaderStorageBlocksCount;
    int32_t maxTesEvaluationShaderStorageBlocksCount;
    int32_t maxTextureBufferSize;
    int32_t maxTextureImageUnitsCount;
    float   maxTextureLODBias;
    int32_t maxTextureSize;
    int32_t maxUniformBufferBindinsCount;
    int32_t maxUniformBlockSize;
    int32_t maxUniformLocationsCount;
    int32_t maxVaryingComponentsCount;
    int32_t maxVaryingVectorsCount;
    int32_t maxVaryingFloatsCount;
    int32_t maxVertexAtomicsCountersCount;
    int32_t maxVertexAttribsCount;
    int32_t maxVertexShaderStorageBlocksCount;
    int32_t maxVertexTextureImageUntisCount;
    int32_t maxVertexUnifromComponentsCount;
    int32_t maxVertexUnifromVectorsCount;
    int32_t maxVertexOutputComponentsCount;
    int32_t maxVertexUniformBlocksCount;
    int32_t maxViewportWidth;
    int32_t maxViewportHeight;
    int32_t maxViewportsCount;
    int32_t compressedTextureFormatsCount;
    int32_t extensionsCount;
    int32_t maxVertexAttribRelativeOffset;
    int32_t maxVertexAttribBindingsCount;
    int32_t minViewportBoundRange;
    int32_t maxViewportBoundRange;
    int32_t maxElementIndex;

    const char* pVendorName;
    const char* pRendererName;
    const char* pHardwareVersionName;
    const char* pShadingLanguageName;
};


static OpenGLGlobalInfo g_globalInfo = {};
static bool g_isInitialized = false;


#define CHECK_DRV_INIT() ENG_ASSERT(engIsOpenGLDriverInitialized(), "OpenGL is not intialized")


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

    glGetIntegerv(GL_MINOR_VERSION, &g_globalInfo.minorVersion);
    glGetIntegerv(GL_MAJOR_VERSION, &g_globalInfo.majorVersion);
    glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxComputeShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxCombinedShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &g_globalInfo.maxComputeUniformBlocksCount);
    glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &g_globalInfo.maxComputeTextureImageUnitsCount);
    glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, &g_globalInfo.maxComputeUniformComponentsCount);
    glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &g_globalInfo.maxComputeAtomicCountersCount);
    glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, &g_globalInfo.maxComputeAtomicCounterBuffersCount);
    glGetIntegerv(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, &g_globalInfo.maxCombinedComputeUniformComponentsCount);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &g_globalInfo.maxComputeWorkGroupInvocationsCount);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &g_globalInfo.maxComputeWorkGroupCountX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &g_globalInfo.maxComputeWorkGroupCountY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &g_globalInfo.maxComputeWorkGroupCountZ);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &g_globalInfo.maxComputeWorkGroupSizeX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &g_globalInfo.maxComputeWorkGroupSizeY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &g_globalInfo.maxComputeWorkGroupSizeZ);
    glGetIntegerv(GL_MAX_DEBUG_GROUP_STACK_DEPTH, &g_globalInfo.maxDebugGroupStackDepth);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &g_globalInfo.max3DTextureSize);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &g_globalInfo.maxArrayTextureLayersCount);
    glGetIntegerv(GL_MAX_CLIP_DISTANCES, &g_globalInfo.maxClipDistancesCount);
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &g_globalInfo.maxColorTextureSamplersCount);
    glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTERS, &g_globalInfo.maxCombinedAtomicCountersCount);
    glGetIntegerv(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, &g_globalInfo.maxCombinedFragmentUniformComponentsCount);
    glGetIntegerv(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, &g_globalInfo.maxCombinedGeometryUniformComponentsCount);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &g_globalInfo.maxCombinedTextureImageUnitsCount);
    glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &g_globalInfo.maxCombinedUniformBlocksCount);
    glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &g_globalInfo.maxCombinedVertexUniformComponentsCount);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &g_globalInfo.maxCubeMapTextureSize);
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &g_globalInfo.maxDepthTextureSamplesCount);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &g_globalInfo.maxDrawBuffersCount);
    glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &g_globalInfo.maxDualSourceDrawBuffersCount);
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &g_globalInfo.maxElementIndicesCount);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &g_globalInfo.maxElementVerticesCount);
    glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &g_globalInfo.maxFragmentAtomicCountersCount);
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxFragmentShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &g_globalInfo.maxFragmentInputComponentsCount);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &g_globalInfo.maxFragmentUniformComponentsCount);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &g_globalInfo.maxFragmentUniformVectorsCount);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &g_globalInfo.maxFragmentUniformBlocksCount);
    glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &g_globalInfo.maxFrameBufferWidth);
    glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &g_globalInfo.maxFrameBufferHeight);
    glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &g_globalInfo.maxFrameBufferLayersCount);
    glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &g_globalInfo.maxFrameSamplesCount);
    glGetIntegerv(GL_MAX_GEOMETRY_ATOMIC_COUNTERS, &g_globalInfo.maxGeometryAtomicCountersCount);
    glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxGeometryShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_GEOMETRY_INPUT_COMPONENTS, &g_globalInfo.maxGeometryInputComponentsCount);
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &g_globalInfo.maxGeometryOutputComponentsCount);
    glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &g_globalInfo.maxGeometryTextureImageUnitsCount);
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &g_globalInfo.maxGeometryUniformBlocksCount);
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &g_globalInfo.maxGeometryUniformComponentsCount);
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &g_globalInfo.maxIntegerSamplesCount);
    glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &g_globalInfo.minMapBufferAlignment);
    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &g_globalInfo.maxRectangleTextureSize);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &g_globalInfo.maxRenderBufferSize);
    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &g_globalInfo.maxSampleMaskWordsCount);
    glGetIntegerv(GL_MAX_SERVER_WAIT_TIMEOUT, &g_globalInfo.maxServerWaitTimeout);
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &g_globalInfo.maxShaderStorageBufferBindingsCount);
    glGetIntegerv(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, &g_globalInfo.maxTesConrolAtomicCountersCount);
    glGetIntegerv(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, &g_globalInfo.maxTesEvaluationAtomicCountersCount);
    glGetIntegerv(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxTesControlShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxTesEvaluationShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &g_globalInfo.maxTextureBufferSize);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &g_globalInfo.maxTextureImageUnitsCount);
    glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &g_globalInfo.maxTextureLODBias);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_globalInfo.maxTextureSize);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &g_globalInfo.maxUniformBufferBindinsCount);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &g_globalInfo.maxUniformBlockSize);
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &g_globalInfo.maxUniformLocationsCount);
    glGetIntegerv(GL_MAX_VARYING_COMPONENTS, &g_globalInfo.maxVaryingComponentsCount);
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &g_globalInfo.maxVaryingVectorsCount);
    glGetIntegerv(GL_MAX_VARYING_FLOATS, &g_globalInfo.maxVaryingFloatsCount);
    glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &g_globalInfo.maxVertexAtomicsCountersCount);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &g_globalInfo.maxVertexAttribsCount);
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &g_globalInfo.maxVertexShaderStorageBlocksCount);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &g_globalInfo.maxVertexTextureImageUntisCount);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &g_globalInfo.maxVertexUnifromComponentsCount);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &g_globalInfo.maxVertexUnifromVectorsCount);
    glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &g_globalInfo.maxVertexOutputComponentsCount);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &g_globalInfo.maxVertexUniformBlocksCount);

    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    g_globalInfo.maxViewportWidth = maxViewportDims[0];
    g_globalInfo.maxViewportHeight = maxViewportDims[1];

    glGetIntegerv(GL_MAX_VIEWPORTS, &g_globalInfo.maxViewportsCount);
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &g_globalInfo.compressedTextureFormatsCount);
    glGetIntegerv(GL_NUM_EXTENSIONS, &g_globalInfo.extensionsCount);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &g_globalInfo.maxVertexAttribRelativeOffset);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &g_globalInfo.maxVertexAttribBindingsCount);

    GLint bounds[2];
    glGetIntegerv(GL_VIEWPORT_BOUNDS_RANGE, bounds);
    g_globalInfo.minViewportBoundRange = bounds[0];
    g_globalInfo.maxViewportBoundRange = bounds[1];

    glGetIntegerv(GL_MAX_ELEMENT_INDEX, &g_globalInfo.maxElementIndex);

    g_globalInfo.pVendorName = (const char*)glGetString(GL_VENDOR);
    g_globalInfo.pRendererName = (const char*)glGetString(GL_RENDERER);
    g_globalInfo.pHardwareVersionName = (const char*)glGetString(GL_VERSION);
    g_globalInfo.pShadingLanguageName = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);    

    g_isInitialized = true;

    return true;
}


bool engIsOpenGLDriverInitialized() noexcept
{
    return g_isInitialized;
}


uint32_t engGetOpenGLMajorVersion() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.majorVersion;
}


uint32_t engGetOpenGLMinorVersion() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.minorVersion;
}

uint32_t engGetOpenGLMaxComputeShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxCombinedShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxComputeUniformBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeUniformBlocksCount;
}


uint32_t engGetOpenGLMaxComputeTextureImageUnitsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeTextureImageUnitsCount;
}


uint32_t engGetOpenGLMaxComputeUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeUniformComponentsCount;
}


uint32_t engGetOpenGLMaxComputeAtomicCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeAtomicCountersCount;
}


uint32_t engGetOpenGLMaxComputeAtomicCounterBuffersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeAtomicCounterBuffersCount;
}


uint32_t engGetOpenGLMaxCombinedComputeUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedComputeUniformComponentsCount;
}


uint32_t engGetOpenGLMaxComputeWorkGroupInvocationsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupInvocationsCount;
}


uint32_t engGetOpenGLMaxComputeWorkGroupCountX() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupCountX;
}


uint32_t engGetOpenGLMaxComputeWorkGroupCountY() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupCountY;
}


uint32_t engGetOpenGLMaxComputeWorkGroupCountZ() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupCountZ;
}


uint32_t engGetOpenGLMaxComputeWorkGroupSizeX() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupSizeX;
}


uint32_t engGetOpenGLMaxComputeWorkGroupSizeY() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupSizeY;
}


uint32_t engGetOpenGLMaxComputeWorkGroupSizeZ() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxComputeWorkGroupSizeZ;
}


uint32_t engGetOpenGLMaxDebugGroupStackDepth() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxDebugGroupStackDepth;
}


uint32_t engGetOpenGLMax3DTextureSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.max3DTextureSize;
}


uint32_t engGetOpenGLMaxArrayTextureLayersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxArrayTextureLayersCount;
}


uint32_t engGetOpenGLMaxClipDistancesCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxClipDistancesCount;
}


uint32_t engGetOpenGLMaxColorTextureSamplersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxColorTextureSamplersCount;
}


uint32_t engGetOpenGLMaxCombinedAtomicCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedAtomicCountersCount;
}


uint32_t engGetOpenGLMaxCombinedFragmentUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedFragmentUniformComponentsCount;
}


uint32_t engGetOpenGLMaxCombinedGeometryUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedGeometryUniformComponentsCount;
}


uint32_t engGetOpenGLMaxCombinedTextureImageUnitsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedTextureImageUnitsCount;
}


uint32_t engGetOpenGLMaxCombinedUniformBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedUniformBlocksCount;
}


uint32_t engGetOpenGLMaxCombinedVertexUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCombinedVertexUniformComponentsCount;
}


uint32_t engGetOpenGLMaxCubeMapTextureSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxCubeMapTextureSize;
}


uint32_t engGetOpenGLMaxDepthTextureSamplesCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxDepthTextureSamplesCount;
}


uint32_t engGetOpenGLMaxDrawBuffersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxDrawBuffersCount;
}


uint32_t engGetOpenGLMaxDualSourceDrawBuffersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxDualSourceDrawBuffersCount;
}


uint32_t engGetOpenGLMaxElementIndicesCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxElementIndicesCount;
}


uint32_t engGetOpenGLMaxElementVerticesCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxElementVerticesCount;
}


uint32_t engGetOpenGLMaxFragmentAtomicCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFragmentAtomicCountersCount;
}


uint32_t engGetOpenGLMaxFragmentShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFragmentShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxFragmentInputComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFragmentInputComponentsCount;
}


uint32_t engGetOpenGLMaxFragmentUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFragmentUniformComponentsCount;
}


uint32_t engGetOpenGLMaxFragmentUniformVectorsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFragmentUniformVectorsCount;
}


uint32_t engGetOpenGLMaxFragmentUniformBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFragmentUniformBlocksCount;
}


uint32_t engGetOpenGLMaxFrameBufferWidth() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFrameBufferWidth;
}


uint32_t engGetOpenGLMaxFrameBufferHeight() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFrameBufferHeight;
}


uint32_t engGetOpenGLMaxFrameBufferLayersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFrameBufferLayersCount;
}


uint32_t engGetOpenGLMaxFrameSamplesCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxFrameSamplesCount;
}


uint32_t engGetOpenGLMaxGeometryAtomicCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryAtomicCountersCount;
}


uint32_t engGetOpenGLMaxGeometryShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxGeometryInputComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryInputComponentsCount;
}


uint32_t engGetOpenGLMaxGeometryOutputComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryOutputComponentsCount;
}


uint32_t engGetOpenGLMaxGeometryTextureImageUnitsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryTextureImageUnitsCount;
}


uint32_t engGetOpenGLMaxGeometryUniformBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryUniformBlocksCount;
}


uint32_t engGetOpenGLMaxGeometryUniformComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxGeometryUniformComponentsCount;
}


uint32_t engGetOpenGLMaxIntegerSamplesCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxIntegerSamplesCount;
}


uint32_t engGetOpenGLMinMapBufferAlignment() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.minMapBufferAlignment;
}


uint32_t engGetOpenGLMaxRectangleTextureSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxRectangleTextureSize;
}


uint32_t engGetOpenGLMaxRenderBufferSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxRenderBufferSize;
}


uint32_t engGetOpenGLMaxSampleMaskWordsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxSampleMaskWordsCount;
}


uint32_t engGetOpenGLMaxServerWaitTimeout() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxServerWaitTimeout;
}


uint32_t engGetOpenGLMaxShaderStorageBufferBindingsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxShaderStorageBufferBindingsCount;
}


uint32_t engGetOpenGLMaxTesConrolAtomicCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTesConrolAtomicCountersCount;
}


uint32_t engGetOpenGLMaxTesEvaluationAtomicCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTesEvaluationAtomicCountersCount;
}


uint32_t engGetOpenGLMaxTesControlShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTesControlShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxTesEvaluationShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTesEvaluationShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxTextureBufferSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTextureBufferSize;
}


uint32_t engGetOpenGLMaxTextureImageUnitsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTextureImageUnitsCount;
}


float engGetOpenGLMaxTextureLODBias() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTextureLODBias;
}


uint32_t engGetOpenGLMaxTextureSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxTextureSize;
}


uint32_t engGetOpenGLMaxUniformBufferBindinsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxUniformBufferBindinsCount;
}


uint32_t engGetOpenGLMaxUniformBlockSize() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxUniformBlockSize;
}


uint32_t engGetOpenGLMaxUniformLocationsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxUniformLocationsCount;
}


uint32_t engGetOpenGLMaxVaryingComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVaryingComponentsCount;
}


uint32_t engGetOpenGLMaxVaryingVectorsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVaryingVectorsCount;
}


uint32_t engGetOpenGLMaxVaryingFloatsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVaryingFloatsCount;
}


uint32_t engGetOpenGLMaxVertexAtomicsCountersCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexAtomicsCountersCount;
}


uint32_t engGetOpenGLMaxVertexAttribsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexAttribsCount;
}


uint32_t engGetOpenGLMaxVertexShaderStorageBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexShaderStorageBlocksCount;
}


uint32_t engGetOpenGLMaxVertexTextureImageUntisCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexTextureImageUntisCount;
}


uint32_t engGetOpenGLMaxVertexUnifromComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexUnifromComponentsCount;
}


uint32_t engGetOpenGLMaxVertexUnifromVectorsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexUnifromVectorsCount;
}


uint32_t engGetOpenGLMaxVertexOutputComponentsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexOutputComponentsCount;
}


uint32_t engGetOpenGLMaxVertexUniformBlocksCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexUniformBlocksCount;
}


void engGetOpenGLMaxViewportDimensions(uint32_t& width, uint32_t& height) noexcept
{
    CHECK_DRV_INIT();
    width = g_globalInfo.maxViewportWidth;
    height = g_globalInfo.maxViewportHeight;
}


uint32_t engGetOpenGLMaxViewportsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxViewportsCount;
}


uint32_t engGetOpenGLCompressedTextureFormatsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.compressedTextureFormatsCount;
}


uint32_t engGetOpenGLExtensionsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.extensionsCount;
}


uint32_t engGetOpenGLMaxVertexAttribRelativeOffset() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexAttribRelativeOffset;
}


uint32_t engGetOpenGLMaxVertexAttribBindingsCount() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxVertexAttribBindingsCount;
}


void engGetOpenGLViewportBoundsRange(int32_t& min, int32_t& max) noexcept
{
    CHECK_DRV_INIT();
    min = g_globalInfo.minViewportBoundRange;
    max = g_globalInfo.maxViewportBoundRange;
}


uint32_t engGetOpenGLMaxElementIndex() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.maxElementIndex;
}


const char* engGetOpenGLVendorName() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.pVendorName;
}


const char* engGetOpenGLRendererName() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.pRendererName;
}


const char* engGetOpenGLHardwareVersionName() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.pHardwareVersionName;
}


const char* engGetOpenGLShadingLanguageName() noexcept
{
    CHECK_DRV_INIT();
    return g_globalInfo.pShadingLanguageName;
}

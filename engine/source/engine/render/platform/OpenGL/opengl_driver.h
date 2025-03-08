#ifndef OPENGL_DRIVER_H
#define OPENGL_DRIVER_H

#include <cstdint>

#include <glad/glad.h>


bool engInitOpenGLDriver() noexcept;
bool engIsOpenGLDriverInitialized() noexcept;

uint32_t engGetOpenGLMajorVersion() noexcept;
uint32_t engGetOpenGLMinorVersion() noexcept;

// Returns the maximum number of active shader storage blocks that may be accessed by a compute shader.
uint32_t engGetOpenGLMaxComputeShaderStorageBlocksCount() noexcept;

// Returns the maximum total number of active shader storage blocks that may be accessed by all active shaders.
uint32_t engGetOpenGLMaxCombinedShaderStorageBlocksCount() noexcept;

// Returns the maximum number of uniform blocks per compute shader (at least 14).
uint32_t engGetOpenGLMaxComputeUniformBlocksCount() noexcept;

// Returns the maximum supported texture image units that can be used to access texture maps from the compute shader (at least 16).
uint32_t engGetOpenGLMaxComputeTextureImageUnitsCount() noexcept;

// Returns the maximum number of individual floating-point, integer, or boolean values that can be held in uniform variable storage for a compute shader (at least 1024).
uint32_t engGetOpenGLMaxComputeUniformComponentsCount() noexcept;

// Returns the maximum number of atomic counters available to compute shaders.
uint32_t engGetOpenGLMaxComputeAtomicCountersCount() noexcept;

// Returns the maximum number of atomic counter buffers that may be accessed by a compute shader.
uint32_t engGetOpenGLMaxComputeAtomicCounterBuffersCount() noexcept;

// Returns the number of words for compute shader uniform variables in all uniform blocks (including default) (at least 1).
uint32_t engGetOpenGLMaxCombinedComputeUniformComponentsCount() noexcept;

// Returns the number of invocations in a single local work group (i.e., the product of the three dimensions) that may be dispatched to a compute shader.
uint32_t engGetOpenGLMaxComputeWorkGroupInvocationsCount() noexcept;

// Returns the maximum number of work groups that may be dispatched to a compute shader in X dimension
uint32_t engGetOpenGLMaxComputeWorkGroupCountX() noexcept;

// Returns the maximum number of work groups that may be dispatched to a compute shader in Y dimension
uint32_t engGetOpenGLMaxComputeWorkGroupCountY() noexcept;

// Returns the maximum number of work groups that may be dispatched to a compute shader in Z dimension
uint32_t engGetOpenGLMaxComputeWorkGroupCountZ() noexcept;

uint32_t engGetOpenGLMaxComputeWorkGroupSizeX() noexcept;
uint32_t engGetOpenGLMaxComputeWorkGroupSizeY() noexcept;
uint32_t engGetOpenGLMaxComputeWorkGroupSizeZ() noexcept;

// Returns the maximum depth of the debug message group stack.
uint32_t engGetOpenGLMaxDebugGroupStackDepth() noexcept;

// Returns a rough estimate of the largest 3D texture that the GL can handle (at least 64).
uint32_t engGetOpenGLMax3DTextureSize() noexcept;

// Returns the value indicates the maximum number of layers allowed in an array texture (at least 256).
uint32_t engGetOpenGLMaxArrayTextureLayersCount() noexcept;

// Returns the maximum number of application-defined clipping distances (at least 8).
uint32_t engGetOpenGLMaxClipDistancesCount() noexcept;

// Returns the maximum number of samples in a color multisample texture.
uint32_t engGetOpenGLMaxColorTextureSamplersCount() noexcept;

// Returns the maximum number of atomic counters available to all active shaders.
uint32_t engGetOpenGLMaxCombinedAtomicCountersCount() noexcept;

// Returns the number of words for fragment shader uniform variables in all uniform blocks (including default) (at least 1).
uint32_t engGetOpenGLMaxCombinedFragmentUniformComponentsCount() noexcept;

// Returns the number of words for geometry shader uniform variables in all uniform blocks (including default) (at least 1).
uint32_t engGetOpenGLMaxCombinedGeometryUniformComponentsCount() noexcept;

// Returns the maximum supported texture image units that can be used to access texture maps from the vertex shader and the fragment processor combined.
// If both the vertex shader and the fragment processing stage access the same texture image unit, 
// then that counts as using two texture image units against this limit (at least 48).
uint32_t engGetOpenGLMaxCombinedTextureImageUnitsCount() noexcept;

// Returns the maximum number of uniform blocks per program (at least 70).
uint32_t engGetOpenGLMaxCombinedUniformBlocksCount() noexcept;

// Returns the number of words for vertex shader uniform variables in all uniform blocks (including default) (at least 1).
uint32_t engGetOpenGLMaxCombinedVertexUniformComponentsCount() noexcept;

// Returns The value gives a rough estimate of the largest cube-map texture that the GL can handle (at least 1024).
uint32_t engGetOpenGLMaxCubeMapTextureSize() noexcept;

// Returns the maximum number of samples in a multisample depth or depth-stencil texture.
uint32_t engGetOpenGLMaxDepthTextureSamplesCount() noexcept;

// Returns the maximum number of simultaneous outputs that may be written in a fragment shader (at least 8).
uint32_t engGetOpenGLMaxDrawBuffersCount() noexcept;

// Returns the maximum number of active draw buffers when using dual-source blending (at least 1).
uint32_t engGetOpenGLMaxDualSourceDrawBuffersCount() noexcept;

// Returns the recommended maximum number of vertex array indices.
uint32_t engGetOpenGLMaxElementIndicesCount() noexcept;

// Returns the recommended maximum number of vertex array vertices.
uint32_t engGetOpenGLMaxElementVerticesCount() noexcept;

// Returns the maximum number of atomic counters available to fragment shaders.
uint32_t engGetOpenGLMaxFragmentAtomicCountersCount() noexcept;

// Returns the maximum number of active shader storage blocks that may be accessed by a fragment shader.
uint32_t engGetOpenGLMaxFragmentShaderStorageBlocksCount() noexcept;

// Returns the maximum number of components of the inputs read by the fragment shader (at least 128).
uint32_t engGetOpenGLMaxFragmentInputComponentsCount() noexcept;

// Returns the maximum number of individual floating-point, integer, or boolean values that can be held in uniform variable storage for a fragment shader (at least 1024).
uint32_t engGetOpenGLMaxFragmentUniformComponentsCount() noexcept;

// Returns the maximum number of individual 4-vectors of floating-point, integer, or boolean values that can be held in uniform variable storage for a fragment shader (at least 256).
uint32_t engGetOpenGLMaxFragmentUniformVectorsCount() noexcept;

// Returns the maximum number of uniform blocks per fragment shader (at least 12).
uint32_t engGetOpenGLMaxFragmentUniformBlocksCount() noexcept;

// Returns the maximum width for a framebuffer that has no attachments (at least 16384).
uint32_t engGetOpenGLMaxFrameBufferWidth() noexcept;

// Returns the maximum height for a framebuffer that has no attachments (at least 16384).
uint32_t engGetOpenGLMaxFrameBufferHeight() noexcept;

// Returns the maximum number of layers for a framebuffer that has no attachments (at least 2048).
uint32_t engGetOpenGLMaxFrameBufferLayersCount() noexcept;

// Returns the maximum samples in a framebuffer that has no attachments (at least 4).
uint32_t engGetOpenGLMaxFrameSamplesCount() noexcept;

// Returns the maximum number of atomic counters available to geometry shaders.
uint32_t engGetOpenGLMaxGeometryAtomicCountersCount() noexcept;

// Returns the maximum number of active shader storage blocks that may be accessed by a geometry shader.
uint32_t engGetOpenGLMaxGeometryShaderStorageBlocksCount() noexcept;

// Returns the maximum number of components of the inputs read by the geometry shader (at least 64).
uint32_t engGetOpenGLMaxGeometryInputComponentsCount() noexcept;

// Returns the maximum number of components of outputs written by a geometry shader (at least 128).
uint32_t engGetOpenGLMaxGeometryOutputComponentsCount() noexcept;

// Returns the maximum supported texture image units that can be used to access texture maps from the geometry shader (at least 16).
uint32_t engGetOpenGLMaxGeometryTextureImageUnitsCount() noexcept;

// Returns the maximum number of uniform blocks per geometry shader (at least 12).
uint32_t engGetOpenGLMaxGeometryUniformBlocksCount() noexcept;

// Returns the maximum number of individual floating-point, integer, or boolean values that can be held in uniform variable storage for a geometry shader (at least 1024).
uint32_t engGetOpenGLMaxGeometryUniformComponentsCount() noexcept;

// Returns the maximum number of samples supported in integer format multisample buffers.
uint32_t engGetOpenGLMaxIntegerSamplesCount() noexcept;

// Returns the minimum alignment in basic machine units of pointers returned fromglMapBuffer and glMapBufferRange. This value must be a power of two and must be at least 64.
uint32_t engGetOpenGLMinMapBufferAlignment() noexcept;

// Returns a rough estimate of the largest rectangular texture that the GL can handle (at least 1024).
uint32_t engGetOpenGLMaxRectangleTextureSize() noexcept;

// Returns the value indicates the maximum supported size for renderbuffers.
uint32_t engGetOpenGLMaxRenderBufferSize() noexcept;

// Returns the maximum number of sample mask words.
uint32_t engGetOpenGLMaxSampleMaskWordsCount() noexcept;

// Returns the maximum glWaitSync timeout interval.
uint32_t engGetOpenGLMaxServerWaitTimeout() noexcept;

// Returns the maximum number of shader storage buffer binding points on the context (at least 8).
uint32_t engGetOpenGLMaxShaderStorageBufferBindingsCount() noexcept;

// Returns the maximum number of atomic counters available to tessellation control shaders.
uint32_t engGetOpenGLMaxTesConrolAtomicCountersCount() noexcept;

// Returns the maximum number of atomic counters available to tessellation evaluation shaders.
uint32_t engGetOpenGLMaxTesEvaluationAtomicCountersCount() noexcept;

// Returns the maximum number of active shader storage blocks that may be accessed by a tessellation control shader.
uint32_t engGetOpenGLMaxTesControlShaderStorageBlocksCount() noexcept;

// Returns the maximum number of active shader storage blocks that may be accessed by a tessellation evaluation shader.
uint32_t engGetOpenGLMaxTesEvaluationShaderStorageBlocksCount() noexcept;

// Returns the maximum number of texels allowed in the texel array of a texture buffer object (at least 65536).
uint32_t engGetOpenGLMaxTextureBufferSize() noexcept;

// Returns the maximum supported texture image units that can be used to access texture maps from the fragment shader (at least 16).
uint32_t engGetOpenGLMaxTextureImageUnitsCount() noexcept;

// Returns the maximum, absolute value of the texture level-of-detail bias (at least 2.0).
float engGetOpenGLMaxTextureLODBias() noexcept;

// Returns a rough estimate of the largest texture that the GL can handle (at least 16).
uint32_t engGetOpenGLMaxTextureSize() noexcept;

// Returns the maximum number of uniform buffer binding points on the context (at least 36).
uint32_t engGetOpenGLMaxUniformBufferBindinsCount() noexcept;

// Returns the maximum size in basic machine units of a uniform block (at least 16384).
uint32_t engGetOpenGLMaxUniformBlockSize() noexcept;

// Returns the maximum number of explicitly assignable uniform locations (at least 1024).
uint32_t engGetOpenGLMaxUniformLocationsCount() noexcept;

// Returns the number components for varying variables (at least 60).
uint32_t engGetOpenGLMaxVaryingComponentsCount() noexcept;

// Returns the number 4-vectors for varying variables (at least 15).
uint32_t engGetOpenGLMaxVaryingVectorsCount() noexcept;

// Returns the maximum number of interpolators available for processing varying variables used by vertex and fragment shaders. 
// This value represents the number of individual floating-point values that can be interpolated;
// varying variables declared as vectors, matrices, and arrays will all consume multiple interpolators (at least 32).
uint32_t engGetOpenGLMaxVaryingFloatsCount() noexcept;

// Returns the maximum number of atomic counters available to vertex shaders.
uint32_t engGetOpenGLMaxVertexAtomicsCountersCount() noexcept;

// Returns the maximum number of 4-component generic vertex attributes accessible to a vertex shader (at least 16).
uint32_t engGetOpenGLMaxVertexAttribsCount() noexcept;

// Returns the maximum number of active shader storage blocks that may be accessed by a vertex shader.
uint32_t engGetOpenGLMaxVertexShaderStorageBlocksCount() noexcept;

// Returns the maximum supported texture image units that can be used to access texture maps from the vertex shader (at least 16).
uint32_t engGetOpenGLMaxVertexTextureImageUntisCount() noexcept;

// Returns the maximum number of individual floating-point, integer, or boolean values that can be held in uniform variable storage for a vertex shader (at least 1024).
uint32_t engGetOpenGLMaxVertexUnifromComponentsCount() noexcept;

// Returns the maximum number of 4-vectors that may be held in uniform variable storage for the vertex shader (at least 256).
uint32_t engGetOpenGLMaxVertexUnifromVectorsCount() noexcept;

// Returns the maximum number of components of output written by a vertex shader (at least 64).
uint32_t engGetOpenGLMaxVertexOutputComponentsCount() noexcept;

// Returns the maximum number of uniform blocks per vertex shader (at least 12).
uint32_t engGetOpenGLMaxVertexUniformBlocksCount() noexcept;

// Returns the maximum supported width and height of the viewport. These must be at least as large as the visible dimensions of the display being rendered to.
void engGetOpenGLMaxViewportDimensions(uint32_t& width, uint32_t& height) noexcept;

// Returns the maximum number of simultaneous viewports that are supported (at least 16).
uint32_t engGetOpenGLMaxViewportsCount() noexcept;

// Returns the number of available compressed texture formats (at least 4).
uint32_t engGetOpenGLCompressedTextureFormatsCount() noexcept;

// Returns the number of extensions supported by the GL implementation for the current context.
uint32_t engGetOpenGLExtensionsCount() noexcept;

// Returns the value containing the maximum offset that may be added to a vertex binding offset.
uint32_t engGetOpenGLMaxVertexAttribRelativeOffset() noexcept;

// Returns the value containing the maximum number of vertex buffers that may be bound.
uint32_t engGetOpenGLMaxVertexAttribBindingsCount() noexcept;

// Returns the the minimum and maximum viewport bounds range. The minimum range should be at least [-32768, 32767].
void engGetOpenGLViewportBoundsRange(int32_t& min, int32_t& max) noexcept;

// Returns the maximum index that may be specified during the transfer of generic vertex attributes to the GL.
uint32_t engGetOpenGLMaxElementIndex() noexcept;

const char* engGetOpenGLVendorName() noexcept;
const char* engGetOpenGLRendererName() noexcept;
const char* engGetOpenGLHardwareVersionName() noexcept;
const char* engGetOpenGLShadingLanguageName() noexcept;

#endif
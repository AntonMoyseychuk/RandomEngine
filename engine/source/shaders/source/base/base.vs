#version 460 core

#include <registers_common.fx>


#if defined(PASS_GBUFFER)
    layout(location = 0) in vec3 vs_in_position;
    layout(location = 1) in vec4 vs_in_color;
    layout(location = 2) in vec3 vs_in_normal;
    layout(location = 3) in vec2 vs_in_texCoords;
#endif


#if defined(PASS_GBUFFER)
    layout(location = 0) out vec4 vs_out_color;
    layout(location = 1) out vec2 vs_out_texCoords;
#elif defined(PASS_MERGE)
    layout(location = 0) out vec2 vs_out_texCoords;
#endif


struct Vertex
{
#if defined(PASS_GBUFFER)
    vec3 position;
    vec4 color;
    vec3 normal;
    vec2 texCoords;
#elif defined(PASS_MERGE)
    vec4 position;
    vec2 texCoords;
#endif
};


#if defined(PASS_MERGE)
    Vertex vertices[6] = Vertex[6](
        Vertex(vec4(-1.0f, -1.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)),
        Vertex(vec4( 1.0f, -1.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec4(-1.0f,  1.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f)),

        Vertex(vec4( 1.0f, -1.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec4( 1.0f,  1.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f)),
        Vertex(vec4(-1.0f,  1.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f))
    );
#endif


void main()
{
#if defined(PASS_GBUFFER)
    vs_out_color = vs_in_color;
    vs_out_texCoords = vs_in_texCoords;

    const mat4 projMatrix = mat4(
        COMMON_PROJ_MATRIX_00, COMMON_PROJ_MATRIX_01, COMMON_PROJ_MATRIX_02, COMMON_PROJ_MATRIX_03,
        COMMON_PROJ_MATRIX_10, COMMON_PROJ_MATRIX_11, COMMON_PROJ_MATRIX_12, COMMON_PROJ_MATRIX_13,
        COMMON_PROJ_MATRIX_20, COMMON_PROJ_MATRIX_21, COMMON_PROJ_MATRIX_22, COMMON_PROJ_MATRIX_23,
        COMMON_PROJ_MATRIX_30, COMMON_PROJ_MATRIX_31, COMMON_PROJ_MATRIX_32, COMMON_PROJ_MATRIX_33
    );

    const mat4 viewMatrix = mat4(
        COMMON_VIEW_MATRIX_00, COMMON_VIEW_MATRIX_01, COMMON_VIEW_MATRIX_02, COMMON_VIEW_MATRIX_03,
        COMMON_VIEW_MATRIX_10, COMMON_VIEW_MATRIX_11, COMMON_VIEW_MATRIX_12, COMMON_VIEW_MATRIX_13,
        COMMON_VIEW_MATRIX_20, COMMON_VIEW_MATRIX_21, COMMON_VIEW_MATRIX_22, COMMON_VIEW_MATRIX_23,
        COMMON_VIEW_MATRIX_30, COMMON_VIEW_MATRIX_31, COMMON_VIEW_MATRIX_32, COMMON_VIEW_MATRIX_33
    );

    gl_Position = projMatrix * viewMatrix * vec4(vs_in_position, 1.f);
#else
    vs_out_texCoords = vertices[gl_VertexID].texCoords;
    gl_Position = vertices[gl_VertexID].position;
#endif
}
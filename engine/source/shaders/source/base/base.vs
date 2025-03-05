#version 460 core

#include <registers_common.fx>
#include <common_math.fx>


#if defined(PASS_GBUFFER)
    layout(location = 0) in vec3 vs_in_position;
    layout(location = 1) in vec3 vs_in_normal;
    layout(location = 2) in vec2 vs_in_texCoords;
#endif


#if defined(PASS_GBUFFER)
    layout(location = 0) out vec3 vs_out_normal;
    layout(location = 1) out vec2 vs_out_texCoords;
#elif defined(PASS_MERGE)
    layout(location = 0) out vec2 vs_out_texCoords;
#endif


struct Vertex
{
#if defined(PASS_GBUFFER)
    vec3 position;
    vec3 normal;
    vec2 texCoords;
#elif defined(PASS_MERGE)
    vec4 position;
    vec2 texCoords;
#endif
};


#if defined(PASS_MERGE)
    Vertex vertices[6] = Vertex[6](
        Vertex(vec4(-1.0f, -1.0f, 0.5f, 1.0f), vec2(0.0f, 0.0f)),
        Vertex(vec4( 1.0f, -1.0f, 0.5f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec4( 1.0f,  1.0f, 0.5f, 1.0f), vec2(1.0f, 1.0f)),

        Vertex(vec4(-1.0f, -1.0f, 0.5f, 1.0f), vec2(0.0f, 0.0f)),
        Vertex(vec4( 1.0f,  1.0f, 0.5f, 1.0f), vec2(1.0f, 1.0f)),
        Vertex(vec4(-1.0f,  1.0f, 0.5f, 1.0f), vec2(0.0f, 1.0f))
    );
#endif


void main()
{
#if defined(PASS_GBUFFER)
    vs_out_normal    = normalize(vs_in_normal);
    vs_out_texCoords = vs_in_texCoords;

    const vec4 wpos = vec4(vs_in_position, 1.0f);
    gl_Position = TransformVec4(wpos, COMMON_VIEW_PROJ_MATRIX);
#else
    vs_out_texCoords = vertices[gl_VertexID].texCoords;
    gl_Position      = vertices[gl_VertexID].position;
#endif
}
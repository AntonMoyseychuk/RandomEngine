#version 460 core


#if defined(PASS_GBUFFER)
    layout(location = 0) out vec4 vs_out_color;
    layout(location = 1) out vec2 vs_out_texCoords;
#elif defined(PASS_MERGE)
    layout(location = 0) out vec2 vs_out_texCoords;
#endif


struct Vertex
{
#if defined(PASS_GBUFFER)
    vec4 position;
    vec4 color;
    vec2 texCoords;
#elif defined(PASS_MERGE)
    vec4 position;
    vec2 texCoords;
#endif
};

#if defined(PASS_GBUFFER)
    Vertex vertices[6] = Vertex[6](
        Vertex(vec4(-0.7f, -0.7f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec2(-2.0f, -2.0f)),
        Vertex(vec4( 0.7f, -0.7f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec2( 2.0f, -2.0f)),
        Vertex(vec4(-0.7f,  0.7f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec2(-2.0f,  2.0f)),

        Vertex(vec4( 0.7f, -0.7f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec2( 2.0f, -2.0f)),
        Vertex(vec4( 0.7f,  0.7f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec2( 2.0f,  2.0f)),
        Vertex(vec4(-0.7f,  0.7f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec2(-2.0f,  2.0f))
    );
#elif defined(PASS_MERGE)
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
    vs_out_color = vertices[gl_VertexID].color;
#endif
    vs_out_texCoords = vertices[gl_VertexID].texCoords;

    gl_Position = vertices[gl_VertexID].position;
}
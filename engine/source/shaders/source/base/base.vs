#version 460 core


layout(location = 0) out vec4 vs_out_color;
layout(location = 1) out vec2 vs_out_texCoords;


struct Vertex
{
    vec4 position;
    vec4 color;
    vec2 texCoords;
};


Vertex vertices[3] = Vertex[3](
    Vertex(vec4(-0.7f, -0.7f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)),
    Vertex(vec4( 0.7f, -0.7f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec4( 0.0f,  0.7f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f))
);


void main()
{
#if defined(_DEBUG)
    vs_out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
#else
    vs_out_color = vertices[gl_VertexID].color;
#endif

    vs_out_texCoords = vertices[gl_VertexID].texCoords;

    gl_Position =  vertices[gl_VertexID].position;
}
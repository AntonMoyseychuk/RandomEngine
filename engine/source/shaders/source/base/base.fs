#version 460 core

#include <registers_common.fx>
#include <common_math.fx>


layout(location = 0) in vec4 fs_in_color;
layout(location = 1) in vec2 fs_in_texCoords;

layout(location = 0) out vec4 fs_out_color;


void main()
{
#if defined(ENV_DEBUG)
    fs_out_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
#else
    fs_out_color = fs_in_color * texture(TEST_TEXTURE, fs_in_texCoords) * abs(sin(COMMON_ELAPSED_TIME * M_PI));
#endif
}
#version 460 core

#include <registers_common.fx>
#include <common_math.fx>


layout(location = 0) in vec4 fs_in_color;

layout(location = 0) out vec4 fs_out_color;


void main()
{
#if defined(ENV_DEBUG)
    fs_out_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
#else
    fs_out_color = fs_in_color * abs(sin(COMMON_ELAPSED_TIME * M_PI));
#endif
}
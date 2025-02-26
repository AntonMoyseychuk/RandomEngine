#version 460 core

#include <registers_common.fx>
#include <common_math.fx>


#if defined(PASS_GBUFFER)
    layout(location = 0) in vec4 fs_in_color;
    layout(location = 1) in vec2 fs_in_texCoords;
#elif defined(PASS_MERGE)
    layout(location = 0) in vec2 fs_in_texCoords;
#endif


#if defined(PASS_GBUFFER)
    layout(location = 0) out vec4 fs_out_albedo;
    layout(location = 1) out vec4 fs_out_normal;
    layout(location = 2) out vec4 fs_out_specular;
#elif defined(PASS_MERGE)
    layout(location = 0) out vec4 fs_out_merge_color;
#endif


void main()
{
#if defined(PASS_GBUFFER)
    const vec4 albedo = texture(TEST_TEXTURE, fs_in_texCoords);

    fs_out_albedo = albedo * abs(sin(COMMON_ELAPSED_TIME));
    fs_out_normal = fs_in_color;
    fs_out_specular = fs_in_color;
#elif defined(PASS_MERGE)
    const vec4 mergeColor = texture(GBUFFER_ALBEDO_TEX, fs_in_texCoords);
    fs_out_merge_color = mergeColor;
#endif
}
#version 460 core

#include <registers_common.fx>
#include <common_math.fx>


#if defined(PASS_GBUFFER)
    layout(location = 0) in vec3 fs_in_normal;
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
    fs_out_normal = vec4(normalize(fs_in_normal), 1.f);
    fs_out_specular = vec4(lerp(vec2(1.f, 0.f), vec2(0.f, 1.f), fs_in_texCoords.x), 0.f, 1.f);
#elif defined(PASS_MERGE)
    vec2 uv = fs_in_texCoords * 2.f;
    
    if (gl_FragCoord.y >= COMMON_SCREEN_HEIGHT * 0.5f) {
        uv.y -= 1.f;
    }

    if (gl_FragCoord.x >= COMMON_SCREEN_WIDTH * 0.5f) {
        uv.x -= 1.f;
    }

    const vec4 albedo   = texture(GBUFFER_ALBEDO_TEX, uv);
    const vec4 normal   = texture(GBUFFER_NORMAL_TEX, uv);
    const vec4 specular = texture(GBUFFER_SPECULAR_TEX, uv);
    const float depth   = texture(COMMON_DEPTH_TEX, uv).r;
    
    const float t = step(COMMON_SCREEN_WIDTH * 0.5f, gl_FragCoord.x);

    if (gl_FragCoord.y > COMMON_SCREEN_HEIGHT * 0.5f) {
        fs_out_merge_color = lerp(normal, albedo, t);
    } else {
        fs_out_merge_color = lerp(vec4(depth, depth, depth, 1.f), specular, t);
    }
#endif
}
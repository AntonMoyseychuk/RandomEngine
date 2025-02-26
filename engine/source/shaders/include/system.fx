#ifndef SYSTEM_H
#define SYSTEM_H


#define DECLARE_CONSTANT(TYPE, NAME, VALUE) \
    const TYPE NAME = VALUE


#define DECLARE_SRV_VARIABLE(TYPE, NAME, LOCATION, DEF_VALUE) \
    layout(location = LOCATION) uniform TYPE NAME = DEF_VALUE


#define DECLARE_SRV_TEXTURE(TYPE, NAME, BINDING, FORMAT, SAMPLER_IDX) \
    layout(binding = BINDING) uniform TYPE NAME


#define DECLARE_CBV(NAME, BINDING) \
    layout(std140, binding = BINDING) uniform NAME


#define REFLECT_INCLUDE(NAME)


#define ZEROF2 vec2(0.f)
#define ZEROF3 vec3(0.f)
#define ZEROF4 vec4(0.f)

#define ONEF2 vec2(1.f)
#define ONEF3 vec3(1.f)
#define ONEF4 vec4(1.f)

#endif
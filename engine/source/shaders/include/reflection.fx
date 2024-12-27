#ifndef SYSTEM_H
#define SYSTEM_H

#define DECLARE_CONSTANT(TYPE, NAME, VALUE)                     const TYPE NAME = VALUE
#define DECLARE_SRV_VARIABLE(TYPE, NAME, LOCATION, DEF_VALUE)   layout(location = LOCATION) uniform TYPE NAME = DEF_VALUE
#define DECLARE_SRV_TEXTURE(TYPE, NAME, LOCATION, BINDING)      layout(location = LOCATION, binding = BINDING) uniform TYPE NAME

#endif
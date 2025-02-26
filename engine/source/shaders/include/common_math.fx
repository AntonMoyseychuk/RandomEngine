#ifndef COMMON_MATH_H
#define COMMON_MATH_H


#define M_PI        (3.14159265359)
#define M_2PI       (2.0f * M_PI)
#define M_HALF_PI   (0.5f * M_PI)


vec3 TransformVec3(in vec4 vec, in vec4 mat[3])
{
    return vec3(dot(vec, mat[0]), dot(vec, mat[1]), dot(vec, mat[2]));
}


vec4 TransformVec4(in vec4 vec, in vec4 mat[4])
{
    return vec4(dot(vec, mat[0]), dot(vec, mat[1]), dot(vec, mat[2]), dot(vec, mat[3]));
}


#endif
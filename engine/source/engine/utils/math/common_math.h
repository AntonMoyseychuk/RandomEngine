#pragma once

#include "core.h"


#ifdef ENG_USE_INVERTED_Z
    #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_query.hpp>
#include <glm/gtx/norm.hpp>


constexpr inline float M3D_EPS     = glm::epsilon<float>();
constexpr inline float M3D_TWO_EPS = 2.f * M3D_EPS;
constexpr inline float M3D_PI      = glm::pi<float>();
constexpr inline float M3D_TWO_PI  = 2.0f * M3D_PI;
constexpr inline float M3D_HALF_PI = 0.5f * M3D_PI;


constexpr inline glm::vec2 M3D_ZEROF2 = glm::vec2(0.f);
constexpr inline glm::vec3 M3D_ZEROF3 = glm::vec3(0.f);
constexpr inline glm::vec4 M3D_ZEROF4 = glm::vec4(0.f);
constexpr inline glm::vec2 M3D_ONEF2  = glm::vec2(1.f);
constexpr inline glm::vec3 M3D_ONEF3  = glm::vec3(1.f);
constexpr inline glm::vec4 M3D_ONEF4  = glm::vec4(1.f);


constexpr inline glm::vec3 M3D_AXIS_X = glm::vec3(1.f, 0.f, 0.f);
constexpr inline glm::vec3 M3D_AXIS_Y = glm::vec3(0.f, 1.f, 0.f);
constexpr inline glm::vec3 M3D_AXIS_Z = glm::vec3(0.f, 0.f, 1.f);


constexpr inline glm::mat3 M3D_MAT3_IDENTITY = glm::identity<glm::mat3>();
constexpr inline glm::mat4 M3D_MAT4_IDENTITY = glm::identity<glm::mat4>();
constexpr inline glm::quat M3D_QUAT_IDENTITY = glm::identity<glm::quat>();


template <typename T>
constexpr inline bool amIsZero(const T& value) noexcept
{
    return glm::length2(value) < M3D_TWO_EPS;
}

constexpr inline bool amIsZero(float value) noexcept
{
    return glm::abs(value) < M3D_EPS;
}


template <typename T>
constexpr inline bool amIsNormalized(const T& value) noexcept
{
    return glm::abs(glm::length2(value) - 1.f) < M3D_EPS;
}


constexpr inline bool amIsNormalized(const glm::quat& quat) noexcept
{
    return std::abs(glm::length(quat) - 1.f) < M3D_EPS;
}


template <typename T>
constexpr inline bool amAreEqual(const T& left, const T& right) noexcept
{
    return glm::all(glm::epsilonEqual(left, right, M3D_EPS));
}


constexpr inline bool amAreEqual(float left, float right) noexcept
{
    return glm::abs(left - right) < M3D_EPS;
}
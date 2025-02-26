#pragma once

#include "core.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_query.hpp>


constexpr inline float M3D_EPS     = glm::epsilon<float>();
constexpr inline float M3D_PI      = glm::pi<float>();
constexpr inline float M3D_TWO_PI  = 2.f * M3D_PI;
constexpr inline float M3D_HALF_PI = 0.5f * M3D_PI;
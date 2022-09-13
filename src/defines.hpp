#pragma once

// I am sure this could be more generic with environment variables or something
#define WORKING_DIR "D:/Documents/OmniVEngine/"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#define MAX_LIGHTS 10
#define MAX_GAME_OBJECTS  10000
#define MAX_CONCURRENT_RENDER_SYSTEMS 10

namespace OmniV
{
    typedef int8_t int8;
    typedef int32_t int32;
    typedef int64_t int64;
    typedef uint8_t uint8;
    typedef uint32_t uint32;
    typedef uint64_t uint64;

    typedef glm::vec2 Vector2f;
    typedef glm::vec3 Vector3f;
    typedef glm::vec4 Vector4f;

    typedef glm::quat Quaternion;

    typedef glm::mat3 Matrix3f;
    typedef glm::mat4 Matrix4f;

    /*constexpr float kPI = 3.14159265358979323846f;
    constexpr float k2PI = 2.f *  3.14159265358979323846f;
    constexpr float kINV_PI = 1.0f / kPI;
	constexpr float kINV_2PI = 1.0f / ( k2PI );
	constexpr float kINV_4PI = 1.0f / ( 4.f * kPI );
	constexpr float kEPSILON = 1e-8f;
	constexpr float kINFINITY = std::numeric_limits<float>::infinity();
    constexpr float kOFFSET = 0.0001f;
    constexpr float kSQRT_TWO = 1.41421356237309504880f;
    constexpr float kINV_SQRT_TWO = 1.f / kSQRT_TWO;*/
};
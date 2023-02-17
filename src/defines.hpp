#pragma once

// Could use precompiled headers

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <memory>
#include <string>
#include <functional>
#include <filesystem>

// logs
#ifndef NDEBUG
    #define OV_DEBUG_LOG(x) std::cout << x << std::endl
    #define OV_DEBUG_ERROR(x) std::cerr << x << std::endl
#else
    #define LOG(x)
    #define LOGERROR(x)
#endif

// global consts
#define MAX_LIGHTS 10
#define MAX_GAME_OBJECTS 10000
#define MAX_CONCURRENT_RENDER_SYSTEMS 10

#define SHADOWMAP_RES 4096
#define SHADOWMAP_MAX_DIST 20
#define SHADOWMAP_CASCADE_COUNT 4
#define SHADOWMAP_CASCADE_LAMBDA 0.95f

// Should be an input from the scene or something
// The actual proper way to do it would be to add behaviours/scripts that the user will create
// and attach to objects in a component system like Unity
#define ROTATE_LIGHTS 1

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
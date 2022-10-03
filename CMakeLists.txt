include(./.libpaths.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
cmake_minimum_required(VERSION 3.11.0)

set(NAME OmniV)
project(${NAME} VERSION 0.1.0)

message(STATUS "")
message(STATUS "Using ${CMAKE_GENERATOR}")

# 1. VulkanSDK
if (DEFINED VULKAN_SDK_PATH)
  message(STATUS "Using Vulkan path specified")
  set(Vulkan_FOUND "True")
else()
  # Find Vulkan outside the project
  find_package(Vulkan REQUIRED QUIET) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
  set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
endif()

set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct

if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using Vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

# 2. GLFW
if (DEFINED GLFW_PATH)
  message(STATUS "Using GLFW path specified")
  set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
  if (MSVC)
    if (CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019")
      set(GLFW_LIB "${GLFW_PATH}/lib-vc2019")
    elseif (CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
      set(GLFW_LIB "${GLFW_PATH}/lib-vc2022")
    else()
      message(FATAL_ERROR "Visual Studio version not supported")
    endif()
  endif()
else()
  # Find GLFW outside the project (NOT TESTED!!)
  find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
  message(STATUS "Found GLFW")
endif()
if (NOT GLFW_LIB)
	message(FATAL_ERROR "Could not find GLFW library!")
else()
	message(STATUS "Using GLFW lib at: ${GLFW_LIB}")
endif()

# 3. GLM
if (DEFINED GLM_PATH)
  message(STATUS "Using GLM path specified")
else()
  message(FATAL_ERROR "Did not specify path for GLM! Automatic finding for this library is not supported yet")
endif()

# 4. TinyOBJ
if (DEFINED TINYOBJ_PATH)
  message(STATUS "Using TinyOBJ path specified")
else()
  message(FATAL_ERROR "Did not specify path for TinyOBJ! Automatic finding for this library is not supported yet")
endif()

# 5. PugiXML
if (DEFINED PUGIXML_PATH)
  message(STATUS "Using PugiXML path specified")
  set(PUGIXML_INCLUDE_DIRS "${PUGIXML_PATH}/src")
  if (MSVC)
    if (CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019")
      set(PUGIXML_LIB "${PUGIXML_PATH}/scripts/vs2019")
    elseif (CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
      set(PUGIXML_LIB "${PUGIXML_PATH}/scripts/vs2022")
    else()
      message(FATAL_ERROR "Visual Studio version not supported")
    endif()
  endif()
else()
  # Find PugiXML outside the project (NOT IMPLEMENTED!!)
  message(FATAL_ERROR "Did not specify path for PugiXML library! Automatic finding for this library is not supported yet")
endif()
if (NOT PUGIXML_LIB)
	message(FATAL_ERROR "Could not find PugiXML library!")
else()
	message(STATUS "Using PugiXML lib at: ${PUGIXML_LIB}")
endif()

include_directories(libs)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.hpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

if (WIN32)
  message(STATUS "CREATING BUILD FOR WINDOWS")

  target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_PATH}
    ${TINYOBJ_PATH}
    ${PUGIXML_INCLUDE_DIRS}
    )

  target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    ${GLFW_LIB}
    ${PUGIXML_LIB}/x64_Debug
    ${PUGIXML_LIB}/x64_Release
  )

  target_link_libraries(${PROJECT_NAME} glfw3 debug pugixml_debug optimized pugixml_release vulkan-1)
elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${PROJECT_SOURCE_DIR}/src
      ${TINYOBJ_PATH}
    )
    target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})
endif()


############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
)

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
    SOURCES ${GLSL_SOURCE_FILES}
)
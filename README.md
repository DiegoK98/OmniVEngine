# OmniVEngine
Vulkan Render Engine. The goal of this personal project is to have a renderer built on Vulkan API, to be able to develop and showcase different shading techniques, as a learning exercise.
This is not a game engine, but I plan on including a GUI layer on top of the current engine, making it more user friendly.
The base of this engine initially based on [this engine](https://github.com/blurrypiano/littleVulkanEngine) by [@blurrypiano] (https://www.github.com/blurrypiano)

## How to use
### Pre-requisites
- Install CMake (minimum version 3.11.0).
- Install Visual Studio (VS2019 or VS2022).
- Install Vulkan SDK.
- Download this repository.
- Edit the gen.bat file if necessary, specifying the IDE you will use. If you are not using VS2019 or VS2022, you will also have to adapt the CMakeLists file (and get the lib files for that IDE).

### Setup
- Execute gen.bat. This will generate the full project inside the build directory.
- Open the project solution and build it (You will also need to build the Shaders target).

## Important notes
- Remember that after any change to the shaders content, you will need to re-build the Shaders target, this will not happen automatically when building the default target.
- If Visual Studio doesn't detect newly created files, executing gen.bat should fix it

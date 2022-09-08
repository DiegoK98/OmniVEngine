# OmniVEngine
Vulkan Render Engine. The goal of this project is to provide an engine where you can develop and showcase many different shading techniques, for research purposes.
This is not a game engine, but I plan on including a GUI layer on top of the current engine, making it easier to create basic games.
The Engine was initially based on the video tutorial series by [blurrypiano](https://github.com/blurrypiano/littleVulkanEngine)

## Setup guide
### Pre-requisites
- Download this repository.
- Install CMake (minimum version 3.11.0).
- Install Visual Studio 2022 (other versions, or even other IDEs can be used, but you will need to change the gen.bat content and CMakeLists accordingly).
- Install Vulkan SDK (.env file contains the path for the SDK version to be used, change it accordingly).

### Setup
- Execute gen.bat. This will generate the project files inside the build directory.
- Open the solution in build.
- Build the engine project in Release mode (Debug doesn't work right now).

## Important notes
- Remember that after any change to the shaders content you will need to build the Shaders target, this will not happen automatically when building the engine project.
- If the visual project doesn't detect newly created files, executing gen.bat should fix it

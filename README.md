# OmniVEngine
Vulkan Render Engine. The goal of this project is to provide an engine where you can develop and showcase many different shading techniques, for research purposes.
This is not a game engine, but I plan on including a GUI layer on top of the current engine, making it easier to create basic games.
The Engine was initially based on the video tutorial series by [blurrypiano](https://github.com/blurrypiano/littleVulkanEngine)

## Setup guide
### Pre-requisites
- Install CMake (minimum version 3.11.0).
- Install Visual Studio (any version newer than VS2012 should be supported, but only VS2017, VS2019 and VS2022 are tested).
- Install Vulkan SDK.
- Download this repository.
- Edit the gen.bat file, specifying the IDE you will use. If you are not using Visual Studio, you will need to adapt the CMakeLists.txt file to your needs (and get the built library files for your IDE).

### Setup
- Execute gen.bat. This will generate the full project inside the build directory.
- Open the project solution with Visual Studio and run it in Release mode (Debug doesn't work right now).

## Important notes
- Remember that after any change to the shaders content, you will need to re-build the Shaders target, this will not happen automatically when building the default target.
- If Visual Studio doesn't detect newly created files, executing gen.bat should fix it

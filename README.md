# Obsidian

Obsidian is a custom game engine built on the Vulkan graphics API. It was created to learn C++ and Vulkan/graphics programming concepts, with the long-term goal of enabling the development of complex games, including open-world titles with procedural generation.


Note: This project is in an early stage and should evolve significantly in the coming months. 

# Acknowledgments

This project was developed as a learning exercise. Currently, with core Vulkan concepts inspired by:
- [Official Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Vulkan C++ playlist by Brendan Galea](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)

# Project structure

```
engine/
├── src/
│ ├── core/ # Application base class
│ ├── Renderer/
│ │ ├── Vulkan/ # Low-level Vulkan abstractions
│ │ └── Common/ # High-level rendering abstractions
│ ├── RendererSystems/ # Specialized rendering systems
│ └── Utils/
sandbox/ # Game-specific implementation
```

# Preview

Here is a preview as of now of a scene - including a 3D model with a texture and lighting:

![screenshot](screenshot.png)

# Key Features and main components

At this stage, Obsidian includes some of the foundational components typical of game engines:
- **Vulkan-based rendering pipeline** with support for textures, mipmaps, swap chains, Abstracts Vulkan device initialization, physical/logical device selection, and queue family management
- **3D renderer**, including a window, 3D model loading, camera systems (orthographic/perspective projections), shaders, and basic rendering utilities.

# Future Plans

Obsidian aims to expand with the following features:
- **User Interface** : traditional game engine-style UI components for editor and in-game interfaces,
- **Entity Component System** for efficient game object management,
- **Compute Shaders** GPU-accelerated computations for simulations and effects,
- **Language Model Integration**: support for AI-driven features, such as dynamic dialogue, procedural storytelling, or NPC behaviors using language models,
- **3D procedural generation support**: tools for voxel-based worlds,
- **Post-processing effects**,
- **Raytracing**: potential addition for advanced lighting.

# Getting started

## Prerequisites

- **Vulkan SDK**: https://vulkan.lunarg.com/
- **CMake**

## Installation & Build

This project has been tested on Windows. It should run on other platforms with minimal modifications (macOS with MoltenVK, Linux).

```bash
git clone --recurse-submodules https://github.com/Drovulook/Obsidian.git

cd Obsidian

# On Windows:

## Configure CMake (generates Visual Studio solution)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

## Compilation
### in Debug mode
cmake --build build --config Debug
# in Release mode for better performance
cmake --build build --config Release
```

## Running the Application
```bash
# in Debug mode
./build/bin/Debug/Obsidian.exe
# in Release Mode
./build/bin/Release/Obsidian.exe
```

# On Linux:

# First time (configuration)
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -sf build/compile_commands.json .

# Compilation
cmake --build build
# Running the Application
./build/bin/Obsidian

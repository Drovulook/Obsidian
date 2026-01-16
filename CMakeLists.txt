cmake_minimum_required(VERSION 3.16)
project(Obsidian)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
# Set Vulkan SDK path
set(VULKAN_SDK_PATH "C:/dev/VulkanSDK/1.4.313.2")

# Try to find Vulkan using find_package first
find_package(Vulkan QUIET)

if(NOT Vulkan_FOUND)
    # Manual Vulkan detection
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib")
    
    # Verify that the files exist
    if(EXISTS "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.h" AND EXISTS "${Vulkan_LIBRARIES}")
        set(Vulkan_FOUND TRUE)
        message(STATUS "Found Vulkan SDK at: ${VULKAN_SDK_PATH}")
    else()
        message(FATAL_ERROR "Vulkan SDK not found at ${VULKAN_SDK_PATH}. Please verify the installation.")
    endif()
else()
    message(STATUS "Found Vulkan using find_package")
endif()

# Add GLFW
add_subdirectory(engine/libraries/glfw-3.4)

# Add GLM
add_subdirectory(engine/libraries/glm-1.0.1)

# Collect all source files from engine/src directory (excluding main.cpp)
file(GLOB_RECURSE ENGINE_SOURCES
    "engine/src/*.cpp"
    "engine/src/*.c"
)

# Remove main.cpp from engine sources as it will be part of the executable
list(REMOVE_ITEM ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/engine/src/main.cpp")

# Collect all header files from engine/src directory
file(GLOB_RECURSE ENGINE_HEADERS
    "engine/src/*.h"
    "engine/src/*.hpp"
)

# Collect all source files from sandbox directory
file(GLOB_RECURSE SANDBOX_SOURCES
    "sandbox/*.cpp"
    "sandbox/*.c"
)

# Collect all header files from sandbox directory
file(GLOB_RECURSE SANDBOX_HEADERS
    "sandbox/*.h"
    "sandbox/*.hpp"
)

# Display found files for debugging
message(STATUS "Found engine source files:")
foreach(source ${ENGINE_SOURCES})
    message(STATUS "  ${source}")
endforeach()

message(STATUS "Found engine header files:")
foreach(header ${ENGINE_HEADERS})
    message(STATUS "  ${header}")
endforeach()

message(STATUS "Found sandbox source files:")
foreach(source ${SANDBOX_SOURCES})
    message(STATUS "  ${source}")
endforeach()

message(STATUS "Found sandbox header files:")
foreach(header ${SANDBOX_HEADERS})
    message(STATUS "  ${header}")
endforeach()

# Create engine library (static library containing all engine functionality)
add_library(ObsidianEngine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}  # Add headers for IDE support
)

# Configure engine library
target_include_directories(ObsidianEngine PUBLIC
    ${Vulkan_INCLUDE_DIRS}
    engine/libraries/glfw-3.4/include
    engine/libraries/glm-1.0.1
    engine/libraries/tinyobjloader
    engine/libraries/stbimage
    engine/libraries/nuklear
    engine/src                # Engine source directory
)

# Define engine path macro for shader loading
target_compile_definitions(ObsidianEngine PRIVATE
    ENGINE_PATH="${CMAKE_SOURCE_DIR}/engine"
)

target_link_libraries(ObsidianEngine PUBLIC
    ${Vulkan_LIBRARIES}
    glfw
    glm
)

# Create executable with main.cpp and sandbox sources
add_executable(${PROJECT_NAME}
    engine/src/core/main.cpp       # Main entry point in engine/src
    ${SANDBOX_SOURCES}
    ${SANDBOX_HEADERS}        # Add sandbox headers for IDE support
)

# Include directories for the executable
target_include_directories(${PROJECT_NAME} PRIVATE
    engine/src                # Engine source directory  
    sandbox                   # Sandbox directory
)

# Link the executable with the engine library
target_link_libraries(${PROJECT_NAME} PRIVATE
    ObsidianEngine           # Link with our engine library
)

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Add custom target for shader compilation
add_custom_target(compile_shaders
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/engine/shaders compile.bat
    COMMENT "Compiling shaders..."
    VERBATIM
)

add_custom_command(TARGET compile_shaders POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/engine/shaders/compiled/nuklear
    COMMAND C:/dev/VulkanSDK/1.4.313.2/Bin/glslc.exe ${CMAKE_SOURCE_DIR}/engine/libraries/nuklear/shaders/demo.vert -o ${CMAKE_SOURCE_DIR}/engine/shaders/compiled/nuklear/demo.vert.spv
    COMMAND C:/dev/VulkanSDK/1.4.313.2/Bin/glslc.exe ${CMAKE_SOURCE_DIR}/engine/libraries/nuklear/shaders/demo.frag -o ${CMAKE_SOURCE_DIR}/engine/shaders/compiled/nuklear/demo.frag.spv
    COMMENT "Compiling Nuklear shaders..."
)

# Make shader compilation a dependency of the engine library
add_dependencies(ObsidianEngine compile_shaders)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
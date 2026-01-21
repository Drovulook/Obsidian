cmake_minimum_required(VERSION 3.16)
project(Obsidian)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")

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

# Find glslc shader compiler
find_program(GLSLC_EXECUTABLE glslc HINTS Vulkan::glslc)
if(NOT GLSLC_EXECUTABLE)
    find_program(GLSLC_EXECUTABLE glslc HINTS $ENV{VULKAN_SDK}/bin)
endif()
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Install Vulkan SDK or shaderc.")
endif()
message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")

# Collect shader files
file(GLOB SHADER_SOURCES
    "${CMAKE_SOURCE_DIR}/engine/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/engine/shaders/*.frag"
    "${CMAKE_SOURCE_DIR}/engine/shaders/*.comp"
)

# Create output directory
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/engine/shaders/compiled)

# Add custom commands for each shader
set(SHADER_OUTPUTS "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT "${CMAKE_SOURCE_DIR}/engine/shaders/compiled/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
    )
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

# Add custom target for shader compilation
add_custom_target(compile_shaders DEPENDS ${SHADER_OUTPUTS})

# Nuklear shaders (if they exist)
if(EXISTS "${CMAKE_SOURCE_DIR}/engine/libraries/nuklear/shaders/demo.vert")
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/engine/shaders/compiled/nuklear)
    add_custom_command(TARGET compile_shaders POST_BUILD
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/engine/libraries/nuklear/shaders/demo.vert -o ${CMAKE_SOURCE_DIR}/engine/shaders/compiled/nuklear/demo.vert.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/engine/libraries/nuklear/shaders/demo.frag -o ${CMAKE_SOURCE_DIR}/engine/shaders/compiled/nuklear/demo.frag.spv
        COMMENT "Compiling Nuklear shaders..."
    )
endif()

# Make shader compilation a dependency of the engine library
add_dependencies(ObsidianEngine compile_shaders)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
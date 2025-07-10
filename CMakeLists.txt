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
add_subdirectory(Libraries/glfw-3.4)

# Add GLM
add_subdirectory(Libraries/glm-1.0.1)

# Create executable
add_executable(${PROJECT_NAME}
    engine/main.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    Libraries/glfw-3.4/include
    Libraries/glm-1.0.1
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${Vulkan_LIBRARIES}
    glfw
    glm
)

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

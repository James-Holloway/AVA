cmake_minimum_required(VERSION 3.30)
# AVA - Another Vulkan Abstraction
project(ava)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
# Vulkan, VulkanMemoryAllocator, GLM, vk-bootstrap, SPIRV-Cross
find_package(Vulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan installation required")
endif ()
find_package(glm REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
FetchContent_Declare(
        fetch_vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG v1.3.296
)
FetchContent_Declare(
        fetch_spirv_cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
        GIT_TAG vulkan-sdk-1.3.296
)
FetchContent_MakeAvailable(fetch_vk_bootstrap fetch_spirv_cross)

# Shader utilities
include(cmake/shaders_glslc.cmake)
include(cmake/shaders_slang.cmake)

# AVA itself
add_subdirectory(ava)
# GLFW Test App
add_subdirectory(app)
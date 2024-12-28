cmake_minimum_required(VERSION 3.30)
# AVA - Another Vulkan Abstraction
project(ava LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

set(VK_BOOTSTRAP_INSTALL ON)

include(FetchContent)
# Vulkan, VulkanMemoryAllocator, vk-bootstrap, SPIRV-Cross
find_package(Vulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan installation required")
endif ()
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)
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
# Examples
add_subdirectory(examples EXCLUDE_FROM_ALL)

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
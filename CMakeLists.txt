cmake_minimum_required(VERSION 3.30)
# AVA - Another Vulkan Abstraction
project(ava LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

set(VK_BOOTSTRAP_INSTALL ON)
set(VMA_HPP_ENABLE_INSTALL ON)

include(FetchContent)
# Vulkan, vk-bootstrap, SPIRV-Cross
find_package(Vulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan installation required")
endif ()
FetchContent_Declare(
        fetch_vma_hpp
        GIT_REPOSITORY https://github.com/YaaZ/VulkanMemoryAllocator-Hpp
        GIT_TAG 5e2b0bebc2f03bc255a1754d4dba135f5971eda8
)
FetchContent_Declare(
        fetch_vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG v1.3.302
)
FetchContent_MakeAvailable(fetch_vma_hpp fetch_vk_bootstrap)

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

install(TARGETS ava
        EXPORT ava
        ARCHIVE DESTINATION lib
        FILE_SET top_headers DESTINATION include/ava
        FILE_SET detail_headers DESTINATION include/ava/
        FILE_SET raii_headers DESTINATION include/ava/
)
install(EXPORT ava
        FILE avaTargets.cmake
        NAMESPACE ava::
        DESTINATION lib/cmake/ava
)

write_basic_package_version_file(
        "avaConfigVersion.cmake"
        VERSION ${ava_VERSION}
        COMPATIBILITY AnyNewerVersion
)
install(FILES
        "cmake/avaConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/avaConfigVersion.cmake"
        DESTINATION lib/cmake/ava
)
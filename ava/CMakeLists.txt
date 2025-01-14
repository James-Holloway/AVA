cmake_minimum_required(VERSION 3.30)

file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
file(GLOB TOP_HEADERS "*.hpp" "*.h")
file(GLOB DETAIL_HEADERS "detail/*.hpp" "detail/*.h")
file(GLOB RAII_HEADERS "raii/*.hpp" "raii/*.h")

add_library(ava STATIC
)
target_sources(ava
        PRIVATE ${SOURCES}
        PUBLIC FILE_SET top_headers TYPE HEADERS FILES ${TOP_HEADERS}
        PUBLIC FILE_SET detail_headers TYPE HEADERS FILES ${DETAIL_HEADERS}
        PUBLIC FILE_SET raii_headers TYPE HEADERS FILES ${RAII_HEADERS}
)

target_link_libraries(ava PUBLIC Vulkan::Vulkan VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp vk-bootstrap::vk-bootstrap spirv-cross-core)
if (${UNIX})
    target_link_libraries(ava PUBLIC ${CMAKE_DL_LIBS})
endif ()
# Include ../ so that ava gets included
target_include_directories(ava PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:/>
)
# Precompile vulkan.hpp
target_precompile_headers(ava PUBLIC "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.hpp")

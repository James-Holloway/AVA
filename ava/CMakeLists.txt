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

target_link_libraries(ava PUBLIC Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator vk-bootstrap::vk-bootstrap spirv-cross-core)
if (${UNIX})
        target_link_libraries(ava PUBLIC ${CMAKE_DL_LIBS})
endif ()
# Include ../ so that ava gets included
target_include_directories(ava PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:/>
)

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
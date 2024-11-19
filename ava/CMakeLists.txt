cmake_minimum_required(VERSION 3.30)

add_library(ava STATIC
        ava.cpp
        ava.hpp
        creation.hpp
        creation.cpp
        vulkan.hpp
        vulkan.cpp
        detail/state.hpp
        detail/vulkan.hpp
        detail/detail.hpp
        detail/debugCallback.hpp
        detail/commandBuffer.hpp
        detail/commandBuffer.cpp
        frame.cpp
        frame.hpp
        commandBuffer.hpp
        commandBuffer.cpp
        detail/shaders.cpp
        detail/shaders.hpp
        detail/renderPass.hpp
        renderPass.cpp
        renderPass.hpp
        detail/utility.cpp
        detail/utility.hpp
        detail/vulkan.cpp
        detail/image.hpp
        image.cpp
        image.hpp
        frameBuffer.cpp
        frameBuffer.hpp
        shaders.cpp
        shaders.hpp
        vao.cpp
        vao.hpp
        detail/vao.hpp
        vertexAttribute.hpp
        detail/reflection.cpp
        detail/reflection.hpp
)
target_link_libraries(ava PUBLIC Vulkan::Vulkan glm::glm GPUOpen::VulkanMemoryAllocator vk-bootstrap::vk-bootstrap spirv-cross-core)
if (${UNIX})
        target_link_libraries(ava PUBLIC ${CMAKE_DL_LIBS})
endif ()
target_include_directories(ava PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
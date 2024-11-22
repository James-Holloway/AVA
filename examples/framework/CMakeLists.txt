cmake_minimum_required(VERSION 3.30)

add_library(framework INTERFACE
        framework.hpp
)
target_link_libraries(framework INTERFACE ava glfw glm::glm)
target_include_directories(framework INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
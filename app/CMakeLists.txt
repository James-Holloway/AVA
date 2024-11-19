cmake_minimum_required(VERSION 3.30)

find_package(glfw3 REQUIRED 3.4)

add_executable(app
        main.cpp
)
target_link_libraries(app PRIVATE ava glfw)
compile_shaders_glslc(app "shaders"
        shaders/test.vert
        shaders/test.frag
)
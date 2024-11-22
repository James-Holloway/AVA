cmake_minimum_required(VERSION 3.30)

add_executable(cube
        main.cpp
)
target_link_libraries(cube PUBLIC framework)
compile_shaders_slang(cube ""
        cube.slang
)

cmake_minimum_required(VERSION 3.30)

add_executable(rayquery
        main.cpp
)
target_link_libraries(rayquery PUBLIC framework)
compile_shaders_slang(rayquery ""
        rayquery.slang
)
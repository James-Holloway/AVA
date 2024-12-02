cmake_minimum_required(VERSION 3.30)

add_executable(raytracing
        main.cpp
)
target_link_libraries(raytracing PUBLIC framework tinyobjloader)
compile_shaders_slang(raytracing ""
        raytracing.slang
        final.slang
)
set_assets(raytracing "" SYMLINK FILES
        sponza_nomat.obj
)
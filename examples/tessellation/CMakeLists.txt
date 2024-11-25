cmake_minimum_required(VERSION 3.30)

add_executable(tessellation
        main.cpp
)
target_link_libraries(tessellation PUBLIC framework tinyobjloader)
compile_shaders_slang(tessellation ""
        tessellation.slang
)
set_assets(tessellation "" SYMLINK FILES
        suzanne_smooth.obj
)

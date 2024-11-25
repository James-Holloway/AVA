cmake_minimum_required(VERSION 3.30)

add_executable(deferred
        main.cpp
)
target_link_libraries(deferred PUBLIC framework tinyobjloader)
compile_shaders_slang(deferred ""
        deferred.slang
        final.slang
)
set_assets(deferred "" SYMLINK FILES
        "assets/apollo_11_command_module.mtl"
        "assets/apollo_11_command_module.obj"
        "assets/Image_0.jpg"
        "assets/Image_1.jpg"
        "assets/Image_2.jpg"
        "assets/Image_3.jpg"
        "assets/Image_4.jpg"
        "assets/Image_5.jpg"
)
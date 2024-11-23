cmake_minimum_required(VERSION 3.30)

add_executable(multisampled-monkeys
        main.cpp
)
target_link_libraries(multisampled-monkeys PUBLIC framework tinyobjloader)
compile_shaders_slang(multisampled-monkeys ""
        multisampled-monkeys.slang
)
set_assets(multisampled-monkeys "" SYMLINK FILES
        suzanne.obj
)
cmake_minimum_required(VERSION 3.30)

add_executable(pushconstants
        main.cpp
)
target_link_libraries(pushconstants PUBLIC framework)
compile_shaders_slang(pushconstants ""
        pushconstants.slang
)

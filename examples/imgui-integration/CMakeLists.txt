cmake_minimum_required(VERSION 3.30)

add_executable(imgui-integration
        main.cpp
)
target_link_libraries(imgui-integration PUBLIC framework imgui)
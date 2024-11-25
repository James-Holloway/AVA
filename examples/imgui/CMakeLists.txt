cmake_minimum_required(VERSION 3.30)

add_library(imgui STATIC
        imconfig.h
        imgui.cpp
        imgui.h
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_internal.h
        imgui_tables.cpp
        imgui_widgets.cpp
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_glfw.h
        backends/imgui_impl_vulkan.cpp
        backends/imgui_impl_vulkan.h
        misc/cpp/imgui_stdlib.cpp
        misc/cpp/imgui_stdlib.h
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
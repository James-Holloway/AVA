cmake_minimum_required(VERSION 3.30)

find_package(glfw3 REQUIRED 3.4)
find_package(glm REQUIRED)

add_subdirectory(framework)
add_subdirectory(cube)
add_subdirectory(app)
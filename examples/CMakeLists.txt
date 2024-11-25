cmake_minimum_required(VERSION 3.30)

find_package(glfw3 REQUIRED 3.4)
find_package(glm REQUIRED)

FetchContent_Declare(
        fetch_tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
        GIT_TAG v2.0.0rc13
)
FetchContent_MakeAvailable(fetch_tinyobjloader)

include(cmake/assets.cmake)
# Framework
add_subdirectory(framework)
# Examples
add_subdirectory(cube)
add_subdirectory(app)
add_subdirectory(multisampled-monkeys)
add_subdirectory(deferred)
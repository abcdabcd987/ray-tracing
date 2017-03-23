cmake_minimum_required(VERSION 3.6)
project(ray_tracing)
set(CMAKE_CXX_STANDARD 11)

include(FindPkgConfig)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(IMGUI_SRC
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/examples/opengl2_example/imgui_impl_glfw.cpp)
include_directories(
        vendor/imgui
        vendor/imgui/examples/libs/gl3w
        vendor/imgui/examples/opengl2_example)

set(SOURCE_CODE
        src/geometry.hpp src/raytracer.hpp src/test_scene.hpp)
add_executable(raytracer-gui src/gui.cpp ${SOURCE_CODE} ${IMGUI_SRC})
target_link_libraries(raytracer-gui glfw ${OPENGL_LIBRARIES})

find_library(FRAMEWORK_CORE_FOUNDATION CoreFoundation)
find_library(FRAMEWORK_OPENGL OpenGl)
target_link_libraries(raytracer-gui ${FRAMEWORK_CORE_FOUNDATION} ${FRAMEWORK_OPENGL})

add_executable(raytracer-cli src/cli.cpp ${SOURCE_CODE})

cmake_minimum_required(VERSION 3.6)
project(ray_tracing)
set(CMAKE_CXX_STANDARD 11)

include(FindPkgConfig)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})

set(IMGUI_SRC
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/examples/libs/gl3w/GL/gl3w.c
        vendor/imgui/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp)
include_directories(
        vendor/imgui
        vendor/imgui/examples/libs/gl3w
        vendor/imgui/examples/sdl_opengl3_example)

set(SOURCE_CODE
        src/main.cpp)
add_executable(ray_tracing ${SOURCE_CODE} ${IMGUI_SRC})
target_link_libraries(ray_tracing ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

find_library(FRAMEWORK_CORE_FOUNDATION CoreFoundation)
find_library(FRAMEWORK_OPENGL OpenGl)
target_link_libraries(ray_tracing ${FRAMEWORK_CORE_FOUNDATION} ${FRAMEWORK_OPENGL})

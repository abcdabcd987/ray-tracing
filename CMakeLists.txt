cmake_minimum_required(VERSION 3.5)
project(ray_tracing)
option(GUI "build with graphical user interface" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3")

include_directories(vendor/concurrentqueue)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

add_executable(raytracer-cli src/cli.cpp ${SOURCE_CODE})
target_link_libraries(raytracer-cli ${PNG_LIBRARY})

if(GUI)
    include(FindPkgConfig)
    cmake_policy(SET CMP0004 OLD) # leading or trailing whitespace????
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
    add_executable(raytracer-gui src/gui.cpp ${IMGUI_SRC})
    target_link_libraries(raytracer-gui ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${PNG_LIBRARY})
    if(APPLE)
        find_library(FRAMEWORK_CORE_FOUNDATION CoreFoundation)
        find_library(FRAMEWORK_OPENGL OpenGl)
        target_link_libraries(raytracer-gui ${FRAMEWORK_CORE_FOUNDATION} ${FRAMEWORK_OPENGL})
    elseif(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
        target_link_libraries(raytracer-gui dl)
    endif()
endif()

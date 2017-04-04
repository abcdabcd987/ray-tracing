cmake_minimum_required(VERSION 3.5)
project(ray_tracing)
option(GUI "build with graphical user interface" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3")

include_directories(vendor/concurrentqueue)

add_executable(raytracer-cli src/cli.cpp ${SOURCE_CODE})

if(GUI)
    include(FindPkgConfig)
    cmake_policy(SET CMP0004 OLD) # leading or trailing whitespace????
    find_package(OpenGL QUIET)
    find_package(SDL2 QUIET)
    if(APPLE)
        set(MESSAGE_OPENGL_NOTFOUND "brew install glfw")
        set(MESSAGE_SDL2_NOTFOUND "brew install sdl2")
    elseif(UNIX)
        set(MESSAGE_OPENGL_NOTFOUND "sudo apt install freeglut3-dev")
        set(MESSAGE_SDL2_NOTFOUND "sudo apt install libsdl2-dev")
    endif()
    set(MESSAGE_NOGUI "Or maybe `cmake -DNOGUI=OFF ..` to disable GUI.")
    if(NOT ${SDL2_FOUND})
        message(FATAL_ERROR "SDL2 not found. Maybe try `${MESSAGE_SDL2_NOTFOUND}`? ${MESSAGE_NOGUI}")
    endif()
    if(NOT ${OpenGL_FOUND})
        message(FATAL_ERROR "OpenGL not found. Maybe try `${MESSAGE_OPENGL_NOTFOUND}`? ${MESSAGE_NOGUI}")
    endif()

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
    target_link_libraries(raytracer-gui ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
    if(APPLE)
        find_library(FRAMEWORK_CORE_FOUNDATION CoreFoundation)
        find_library(FRAMEWORK_OPENGL OpenGl)
        target_link_libraries(raytracer-gui ${FRAMEWORK_CORE_FOUNDATION} ${FRAMEWORK_OPENGL})
    elseif(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
        target_link_libraries(raytracer-gui dl)
    endif()
endif()

#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
#include <cstdio>
#include <GL/gl3w.h>
#include <SDL.h>
#include <functional>
#include "raytracer.hpp"

int main(int argc, char** argv)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window *window = SDL_CreateWindow("ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(window);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154).Value;

    const auto scale = ImGui::GetIO().DisplayFramebufferScale;
    const int width = 400, height = 300;
    const int render_width = static_cast<int>(width * scale.x);
    const int render_height = static_cast<int>(height * scale.y);
    uint8_t data[render_width * render_height * 3];
    memset(data, 0, sizeof(data));
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Plane p1(Vector3(0, 1, 0), 4.4f);
    p1.material.k_reflect = 0;
    p1.material.k_diffuse = 1;
    p1.material.color = Color(0.4f, 0.3f, 0.3f);
    Sphere p2(Vector3(1, -0.8f ,3), 2.5f);
    p2.material.k_reflect = 0.6;
    p2.material.color = Color(0.7f, 0.7f, 0.7f);
    Sphere p3(Vector3(-5.5f, -0.5f, 7.0f), 2);
    p3.material.k_reflect = 1.0f;
    p3.material.k_diffuse = 0.1f;
    p3.material.color = Color(0.7f, 0.7f, 1.0f);
    Sphere p4(Vector3(0, 5, 5), 0.1f);
    p4.light = 1;
    p4.material.color = Color(0.6f, 0.6f, 0.6f);
    Sphere p5(Vector3(2, 5, 1), 0.1f);
    p5.light = 1;
    p5.material.color = Color(0.7f, 0.7f, 0.9f);
    RayTracer tracer;
    tracer.scene.primitives.emplace_back(&p1);
    tracer.scene.primitives.emplace_back(&p2);
    tracer.scene.primitives.emplace_back(&p3);
    tracer.scene.primitives.emplace_back(&p4);
    tracer.scene.primitives.emplace_back(&p5);

    tracer.render(data, width, height);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_width, render_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdlGL3_NewFrame(window);

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        {
            static const std::function<void (int, int, uint8_t&, uint8_t&, uint8_t&)> funcs[] = {
                    [](int x, int y, uint8_t& r, uint8_t& g, uint8_t& b) {
                        r = static_cast<uint8_t>((x + y) % 255);
                        g = static_cast<uint8_t>((x - y) % 255);
                        b = static_cast<uint8_t>((x * y) % 255);
                    },
                    [](int x, int y, uint8_t& r, uint8_t& g, uint8_t& b) {
                        r = static_cast<uint8_t>((x - y) % 255);
                        g = static_cast<uint8_t>((x + y) % 255);
                        b = static_cast<uint8_t>((x * y) % 255);
                    },
            };
            static int state = 0;

            ImGui::Begin("Image", &show_another_window);
            if (ImGui::Button("Toggle")) {
                const auto& func = funcs[state];
                for (int y = 0, i = 0; y < render_height; ++y)
                    for (int x = 0; x < render_width; ++x, i += 3)
                        func(x, y, data[i], data[i+1], data[i+2]);
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_width, render_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                state = (state + 1) % (sizeof(funcs) / sizeof(*funcs));
            }
            ImTextureID texid = reinterpret_cast<ImTextureID>(tex);
            ImGui::Image(texid, ImVec2(width, height));
            ImGui::End();
        }

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

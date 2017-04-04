#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
#include <cstdio>
#include <GL/gl3w.h>
#include <SDL.h>
#include <thread>
#include "raytracer.hpp"
#include "test_scene.hpp"

int width, height, render_width, render_height, image_width, image_height;
uint8_t *data;
std::chrono::high_resolution_clock::time_point time_render_start, time_render_end;
enum RenderStatus {WAIT_TO_RENDER, RENDERING, RENDERED, EXIT_RENDER} status;
RayTracer::TraceConfig config;
RayTracer tracer;
GLuint tex;


void show_image_window() {
    ImGui::Begin("Image");
    auto end = status == RENDERING ? std::chrono::high_resolution_clock::now() : time_render_end;
    double sec = (end - time_render_start).count() / 1e9;
    ImGui::Text("render size: %d x %d", image_width, image_height);
    ImGui::Text("rendered %d/%d pixels in %.3fs", tracer.cnt_rendered.load(), image_width * image_height, sec);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    ImVec2 tex_screen_pos = ImGui::GetCursorScreenPos();
    ImTextureID texid = reinterpret_cast<ImTextureID>(tex);
    ImGui::Image(texid, ImVec2(width, height));
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        float focus_sz = 32.0f;
        float focus_x = ImGui::GetMousePos().x - tex_screen_pos.x - focus_sz * 0.5f; if (focus_x < 0.0f) focus_x = 0.0f; else if (focus_x > width  - focus_sz) focus_x = width  - focus_sz;
        float focus_y = ImGui::GetMousePos().y - tex_screen_pos.y - focus_sz * 0.5f; if (focus_y < 0.0f) focus_y = 0.0f; else if (focus_y > height - focus_sz) focus_y = height - focus_sz;
        ImGui::Text("Min: (%.2f, %.2f)", focus_x, focus_y);
        ImGui::Text("Max: (%.2f, %.2f)", focus_x + focus_sz, focus_y + focus_sz);
        ImVec2 uv0 = ImVec2((focus_x) / width, (focus_y) / height);
        ImVec2 uv1 = ImVec2((focus_x + focus_sz) / width, (focus_y + focus_sz) / height);
        ImGui::Image(texid, ImVec2(128,128), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
        ImGui::EndTooltip();
    }
    ImGui::End();
}


void show_toolbox_info() {
    ImGui::Text("GUI FPS: %.1f", ImGui::GetIO().Framerate);
}


void show_toolbox_render() {
    int wh[] = {width, height};
    ImGui::InputInt2("width x height", wh);
    width = wh[0], height = wh[1];

    int rwh[] = {render_width, render_height};
    ImGui::InputInt2("render_width x render_height", rwh);
    render_width = rwh[0], render_height = rwh[1];

    ImGui::SliderInt("num_trace_depth", &config.num_trace_depth, 1, 10);
    ImGui::SliderFloat("num_light_sample_per_unit", &config.num_light_sample_per_unit, 1.f, 2000.f);
    ImGui::SliderInt("num_diffuse_reflect_sample", &config.num_diffuse_reflect_sample, 1, 128);
    ImGui::SliderInt("workers", &config.num_worker, 1, std::thread::hardware_concurrency());

    if (ImGui::Button("render")) status = WAIT_TO_RENDER;
    ImGui::SameLine();
    if (ImGui::Button("stop")) tracer.stop();
}


void show_toolbox_scene() {
    static char buf[100], buf2[100];
    for (size_t i = 0; i < tracer.scene.primitives.size(); ++i) {
        Primitive *p = tracer.scene.primitives[i];
        bool open;
        if (p->light)
            sprintf(buf2, "light %d samples", p->get_num_light_sample(config.num_light_sample_per_unit));
        else
            buf2[0] = 0;
        if (Sphere *sphere = dynamic_cast<Sphere*>(p)) {
            sprintf(buf, "%zu: Sphere %s###scene-primitive-%zu", i, buf2, i);
            if ((open = ImGui::TreeNode(buf))) {
                ImGui::DragFloat3("center", sphere->center.data, 0.01f);
                ImGui::DragFloat("radius", &sphere->radius, 0.001f);
            }
        } else if (Box *box = dynamic_cast<Box*>(p)) {
            sprintf(buf, "%zu: Box %s###scene-primitive-%zu", i, buf2, i);
            if ((open = ImGui::TreeNode(buf))) {
                ImGui::DragFloat3("pos", box->aabb.pos.data, 0.01f);
                ImGui::DragFloat3("size", box->aabb.size.data, 0.01f);
            }
        } else if (Triangle *triangle = dynamic_cast<Triangle*>(p)) {
            sprintf(buf, "%zu: Triangle %s###scene-primitive-%zu", i, buf2, i);
            if ((open = ImGui::TreeNode(buf))) {
                Vector3 v0 = triangle->v0, v1 = triangle->v1, v2 = triangle->v2;
                ImGui::DragFloat3("vertex 0", v0.data, 0.01f);
                ImGui::DragFloat3("vertex 1", v1.data, 0.01f);
                ImGui::DragFloat3("vertex 2", v2.data, 0.01f);
                triangle->set_vertices(v0, v1, v2);
            }
        } else if (Plane *plane = dynamic_cast<Plane*>(p)) {
            sprintf(buf, "%zu: Plane %s###scene-primitive-%zu", i, buf2, i);
            if ((open = ImGui::TreeNode(buf))) {
                ImGui::DragFloat3("normal", plane->normal.data, 0.001f);
                ImGui::DragFloat("distance", &plane->distance, 0.01f);
            }
        } else {
            sprintf(buf, "%zu: Primitive %s###scene-primitive-%zu", i, buf2, i);
            open = ImGui::TreeNode(buf);
        }
        if (open) {
            bool light = p->light;
            ImGui::Checkbox("light", &light);
            if (light != p->light) {
                auto &lights = tracer.scene.lights;
                if (p->light) lights.erase(std::remove(lights.begin(), lights.end(), p));
                else lights.emplace_back(p);
                p->light = light;
            }
            ImGui::ColorEdit3("color", p->material.color.data);
            ImGui::SliderFloat("k_reflect", &p->material.k_reflect, 0, 1);
            ImGui::SliderFloat("k_diffuse", &p->material.k_diffuse, 0, 1);
            ImGui::SliderFloat("k_diffuse_reflect", &p->material.k_diffuse_reflect, 0, 1);
            ImGui::SliderFloat("k_specular", &p->material.k_specular, 0, 1);
            ImGui::SliderFloat("k_refract", &p->material.k_refract, 0, 1);
            ImGui::SliderFloat("k_refract_index", &p->material.k_refract_index, 0, 1.5f);
            ImGui::SliderFloat("k_ambient", &p->material.k_ambient, 0, 1);
            ImGui::TreePop();
        }
    }
    ImGui::Separator();

    if (ImGui::Button("new Sphere")) {
        Sphere *sphere = new Sphere(Vector3(0, 0, 0), 0);
        tracer.scene.add(sphere);
    }
    ImGui::SameLine();
    if (ImGui::Button("new Box")) {
        Box *box = new Box(AABB(Vector3(0, 0, 0), Vector3(0, 0, 0)));
        tracer.scene.add(box);
    }
    ImGui::SameLine();
    if (ImGui::Button("new Plane")) {
        Plane *plane = new Plane(Vector3(1, 0, 0), 0);
        tracer.scene.add(plane);
    }
    ImGui::SameLine();
    if (ImGui::Button("new Triangle")) {
        Triangle *triangle = new Triangle(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0));
        tracer.scene.add(triangle);
    }
}


void show_toolbox_body() {
    static char buf[100];
    static float scale = 1.f, offset[3] = {0, 0, 0};
    for (size_t i = 0; i < tracer.scene.bodies.size(); ++i) {
        Body *body = tracer.scene.bodies[i];
        sprintf(buf, "Body %zu###body-%zu", i, i);
        if (ImGui::TreeNode(buf)) {
            ImGui::DragFloat("", &scale, 0.001f);
            ImGui::SameLine();
            if (ImGui::Button("scale")) {
                body->scale(scale);
                scale = 1.f;
            }

            ImGui::DragFloat3("", offset, 0.01f);
            ImGui::SameLine();
            if (ImGui::Button("offset")) {
                body->offset(Vector3(offset[0], offset[1], offset[2]));
                offset[0] = 0, offset[1] = 0, offset[2] = 0;
            }

            ImGui::TreePop();
        }
    }
}


void show_toolbox_window() {
    ImGui::Begin("Toolbox");
    show_toolbox_info();
    if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) show_toolbox_render();
    if (ImGui::CollapsingHeader("Scene")) show_toolbox_scene();
    if (ImGui::CollapsingHeader("Body")) show_toolbox_body();
    ImGui::End();
}


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
    SDL_Window *window = SDL_CreateWindow("https://github.com/abcdabcd987/ray-tracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
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
    ImVec4 clear_color = ImColor(114, 144, 154).Value;

    const auto scale = ImGui::GetIO().DisplayFramebufferScale;
    width = 800, height = 600;
//    render_width = static_cast<int>(width * scale.x);
//    render_height = static_cast<int>(height * scale.y);
    render_width = 800;
    render_height = 600;
    data = new uint8_t[render_width * render_height * 3];
    memset(data, 0, sizeof(*data) * render_width * render_height * 3);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_width, render_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


    add_scene2(tracer);
    config.num_light_sample_per_unit = 56.f;
    config.num_trace_depth = 4;
    config.num_diffuse_reflect_sample = 1;
    config.num_worker = std::thread::hardware_concurrency();
    status = WAIT_TO_RENDER;

    std::thread render_thread([&]{
        while (status != EXIT_RENDER) {
            if (status == WAIT_TO_RENDER) {
                status = RENDERING;
                if (render_width != image_width || render_height != image_height) {
                    uint8_t *olddata = data;
                    data = new uint8_t[render_width * render_height * 3];
                    image_width = render_width;
                    image_height = render_height;
                    delete [] olddata;
                }
                time_render_start = std::chrono::high_resolution_clock::now();
                bool success = tracer.render(data, image_width, image_height, config);
                time_render_end = std::chrono::high_resolution_clock::now();
                if (success)
                    save_ppm("/tmp/ray-tracing.ppm", data, width, height);
                else if (status == EXIT_RENDER)
                    return;
                status = RENDERED;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    });

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

//        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
//        if (show_test_window)
//        {
//            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
//            ImGui::ShowTestWindow(&show_test_window);
//        }

        show_image_window();
        show_toolbox_window();

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }
    status = EXIT_RENDER;
    tracer.stop();

    // Cleanup
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    render_thread.join();
    delete [] data;

    return 0;
}

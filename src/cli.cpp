#include "raytracer.hpp"
#include "test_scene.hpp"

int main(int argc, char** argv) {
    const int width = 400, height = 300;
    uint8_t data[width * height * 3];
    memset(data, 0, sizeof(data));

    RayTracer tracer;
    add_scene1(tracer);
    RayTracer::TraceConfig config;
    tracer.render(data, width, height, config);
    save_ppm("/tmp/ray-tracing.ppm", data, width, height);
}

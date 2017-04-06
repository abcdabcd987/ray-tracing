#include <string>
#include <fstream>
#include <iomanip>
#include "raytracer.hpp"
#include "test_scene.hpp"

void help() {
    fputs("usage: ./raytracer-cli [options]\n", stderr);
    fputs("options:\n", stderr);
    fputs("   -w <INT>        image width\n", stderr);
    fputs("   -h <INT>        image height\n", stderr);
    fputs("   -d <INT>        ray tracing depth\n", stderr);
    fputs("   -r <INT>        number of diffuse reflect samples\n", stderr);
    fputs("   -l <FLOAT>      number of light samples per unit volume\n", stderr);
    fputs("   -j <INT>        number of thread workers\n", stderr);
    fputs("   -o <STRING>     path to output png image\n", stderr);
    fputs("   -f <STRING>     path to scene json\n", stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int width = 800, height = 600;
    const char *out = "/tmp/ray-tracing.ppm";
    const char *filename;
    RayTracer::TraceConfig config;

    if (argc % 2 != 1 || argc == 1) help();
    for (int i = 1; i < argc; i += 2) {
        std::string key = argv[i];
        const char *value = argv[i+1];
        if (key == "-w") {
            width = std::atoi(value);
        } else if (key == "-h") {
            height = std::atoi(value);
        } else if (key == "-d") {
            config.num_trace_depth = std::atoi(value);
        } else if (key == "-r") {
            config.num_diffuse_reflect_sample = std::atoi(value);
        } else if (key == "-l") {
            config.num_light_sample_per_unit = static_cast<float>(std::atof(value));
        } else if (key == "-j") {
            config.num_worker = std::atoi(value);
        } else if (key == "-o") {
            out = value;
        } else if (key == "-f") {
            filename = value;
        } else {
            fprintf(stderr, "unknown option %s\n", argv[i]);
        }
    }

    uint8_t *data = new uint8_t[width * height * 3];
    memset(data, 0, sizeof(*data) * (width * height * 3));
    RayTracer tracer;
    std::ifstream fin(filename);
    json j;
    fin >> j;
    tracer.scene.from_json(j);

    int cnt_primitive = static_cast<int>(tracer.scene.primitives.size());
    int cnt_triangle = 0;
    for (Body *body : tracer.scene.bodies)
        cnt_triangle += body->triangles.size();
    printf("========== scene information ==========\n");
    printf("                primitives    %d\n", cnt_primitive);
    printf("                 triangles    %d\n", cnt_triangle);
    printf("=========== render settings ===========\n");
    printf("                     width    %d\n", width);
    printf("                    height    %d\n", height);
    printf("               trace depth    %d\n", config.num_trace_depth);
    printf("   diffuse reflect samples    %d\n", config.num_diffuse_reflect_sample);
    printf("  light samples per volume    %.3f\n", config.num_light_sample_per_unit);
    printf("                   workers    %d\n", config.num_worker);

    tracer.render(data, width, height, config);
    save_png(out, data, width, height);
    delete [] data;
}

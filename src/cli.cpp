#include <string>
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
    fputs("   -o <STRING>     output path\n", stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int width = 800, height = 600;
    const char *out = "/tmp/ray-tracing.ppm";
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
        } else {
            fprintf(stderr, "unknown option %s\n", argv[i]);
        }
    }

    uint8_t *data = new uint8_t[width * height * 3];
    memset(data, 0, sizeof(*data) * (width * height * 3));
    RayTracer tracer;
    add_scene2(tracer);
    tracer.render(data, width, height, config);
    save_ppm(out, data, width, height);
    delete [] data;
}

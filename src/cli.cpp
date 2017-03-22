#include "raytracer.hpp"
#include <cstdio>

void save_ppm(const char *path, uint8_t *data, int width, int height) {
    FILE *f = fopen(path, "w");
    fprintf(f, "P3\n");
    fprintf(f, "%d %d\n", width, height);
    fprintf(f, "255\n");
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = (y * width + x) * 3;
            fprintf(f, "%u\t%u\t%u\t", data[i], data[i+1], data[i+2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char** argv) {
    const int width = 400, height = 300;
    uint8_t data[width * height * 3];
    memset(data, 0, sizeof(data));

    const int trace_depth = 6;
    RayTracer tracer;

    Material def_material = {
            .color = Color(.2f, .2f, .2f),
            .k_specular = .8f,
            .k_ambient = 0,
            .k_refract_index = 1.5f,
            .k_refract = 0,
            .k_diffuse = 0.2f,
            .k_reflect = 0,
            .k_diffuse_reflect = 0,
    };

    // ground plane
    Primitive *p0 = new Plane(Vector3(0, 1, 0), 4.4f);
    p0->material = def_material;
    p0->material.k_reflect = 0.0f;
    p0->material.k_refract = 0.0f;
    p0->material.k_diffuse = 1.0f;
    p0->material.color = Color(0.4f, 0.3f, 0.3f);
    // big sphere
    Primitive *p1 = new Sphere(Vector3(0, -0.8f, 7), 2);
    p1->material = def_material;
    p1->material.k_reflect = 0.2f;
    p1->material.k_refract = 0.8f;
    p1->material.k_refract_index = 1.3f;
    p1->material.color = Color(0.7f, 0.7f, 1.0f);
    p1->material.k_diffuse_reflect = 0.3f;
    // small sphere
    Primitive *p2 = new Sphere(Vector3(-5, -0.8f, 7), 2);
    p2->material = def_material;
    p2->material.k_reflect = 0.5f;
    p2->material.k_refract = 0.0f;
    p2->material.k_refract_index = 1.3f;
    p2->material.k_diffuse = 0.1f;
    p2->material.color = Color(0.7f, 0.7f, 1.0f);
    p2->material.k_diffuse_reflect = 0.6f;
    // third sphere
    Primitive *p3 = new Sphere(Vector3(5, -0.8f, 7), 2);
    p3->material = def_material;
    p3->material.k_reflect = 0.5f;
    p3->material.k_refract = 0.0f;
    p3->material.k_refract_index = 1.3f;
    p3->material.k_diffuse = 0.1f;
    p3->material.color = Color(0.7f, 0.7f, 1.0f);
#if 1
    // area light
    Primitive *p4 = new Box(AABB(Vector3(-1, 5, 4), Vector3( 2, 0.1f, 2)));
    p4->material = def_material;
    p4->light = 1;
    p4->material.color = Color(1, 1, 1);
#else
    // light source 1
    Primitive *p4 = new Sphere(Vector3(0, 5, 5), 0.1f);
    p4->material = def_material;
    p4->light = 1;
    p4->material.color = Color(1, 1, 1);
#endif
    // back plane
    Primitive *p5 = new Plane(Vector3(0, 0, -1), 12.4f);
    p5->material = def_material;
    p5->material.k_reflect = 0.0f;
    p5->material.k_refract = 0.0f;
    p5->material.k_diffuse = 1.0f;
    p5->material.color = Color(0.4f, 0.3f, 0.3f);
    // ceiling
    Primitive *p6 = new Plane(Vector3(0, -1, 0), 5.2f);
    p6->material = def_material;
    p6->material.k_reflect = 0.0f;
    p6->material.k_refract = 0.0f;
    p6->material.k_diffuse = 1.0f;
    p6->material.color = Color(0.4f, 0.3f, 0.3f);


    tracer.scene.primitives.emplace_back(p0);
    tracer.scene.primitives.emplace_back(p1);
    tracer.scene.primitives.emplace_back(p2);
    tracer.scene.primitives.emplace_back(p3);
    tracer.scene.primitives.emplace_back(p4);
    tracer.scene.primitives.emplace_back(p5);
    tracer.scene.primitives.emplace_back(p6);


    tracer.render(data, width, height, trace_depth);
    save_ppm("/tmp/ray-tracing.ppm", data, width, height);
}

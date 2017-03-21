#pragma once
#include <limits>
#include <vector>
#include <cstdint>
#include <cassert>
#include "geometry.hpp"

struct RayTracer {
    Scene scene;

    RayTracer(): scene() {}

    struct RayTraceResult {
        bool hit;
        float dist;
        Color color;
        const Primitive *primitive;
    };

    RayTraceResult ray_trace(const Ray& ray, int remain_depth) const {
        if (!remain_depth) return {.hit = false};

        // find the nearest intersection
        float dist = std::numeric_limits<float>::max();
        const Primitive *p = nullptr;
        for (const Primitive *pr : scene.primitives) {
            auto res = pr->intersect(ray);
            if (res.hit != IntersectionResult::MISS && res.distance < dist) {
                dist = res.distance;
                p = pr;
            }
        }

        // if no intersection
        if (!p) return {.hit = false};

        // if light
        if (p->light) return {.hit = true, .dist = dist, .color = Color(1.f, 1.f, 1.f), .primitive = p};

        // if normal object
        Vector3 pi = ray.origin + ray.direction * dist; // intersection point
        for (const Primitive *pr : scene.primitives) {
            if (!pr->light) continue;
            // diffuse shading
            // TODO: light sources other than Sphere
            // TODO: path to the light source is blocked
            const Sphere *light = dynamic_cast<const Sphere*>(pr);
            assert(light);
            Vector3 L = (light->center - pi).normalized();
            Vector3 N = p->get_normal(pi);
            if (p->material.k_diffuse > 0) {
                float dot = N.dot(L);
                if (dot > 0) {
                    float diff = dot * p->material.k_diffuse;
                    Color c = diff * p->material.color * light->material.color;
                    return {.hit = true, .dist = dist, .color = c, .primitive = p};
                }
            }
        }

        // otherwise
        return {.hit = false};
    }

    // TODO: camera position, screen position, z direction
    void render(uint8_t *out, int width, int height) {
        float wx1 = -4, wx2 = 4, wy1 = 3, wy2 = -3;
        float dx = (wx2 - wx1) / width;
        float dy = (wy2 - wy1) / height;
        Vector3 o(0, 0, -5);
        for (int y = 0; y < height; ++y) {
            const float sy = wy1 + dy * y;
            for (int x = 0; x < width; ++x) {
                const float sx = wx1 + dx * x;
                Ray ray(o, Vector3(sx, sy, 0) - o);
                RayTraceResult res = ray_trace(ray, 1);
                if (res.hit) {
                    const int idx = (y * width + x) * 3;
                    color_add_to_array(&out[idx], res.color);
                }
            }
        }
    }
};

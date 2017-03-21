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
        RayTraceResult res = {.hit = false, .dist = 0, .color = Color(0, 0, 0), .primitive = nullptr};
        if (!remain_depth) return res;

        // find the nearest intersection
        float dist = std::numeric_limits<float>::max();
        for (const Primitive *pr : scene.primitives) {
            auto res_intersect = pr->intersect(ray);
            if (res_intersect.hit != IntersectionResult::MISS && res_intersect.distance < dist) {
                dist = res_intersect.distance;
                res.primitive = pr;
            }
        }

        // if no intersection
        if (!res.primitive) return res;

        // if light
        if (res.primitive->light) return {.hit = true, .dist = dist, .color = Color(1.f, 1.f, 1.f), .primitive = res.primitive};

        // if normal object
        res.hit = true;
        res.dist = dist;
        Vector3 pi = ray.origin + ray.direction * dist; // intersection point
        Vector3 N = res.primitive->get_normal(pi);
        for (const Primitive *pr : scene.primitives) {
            if (!pr->light) continue;
            // TODO: light sources other than Sphere
            const Sphere *light = dynamic_cast<const Sphere*>(pr);
            assert(light);
            Vector3 L = light->center - pi;
            float dist_light = L.length();
            L = L.normalized();

            // shadow
            float shade = 1.0f;
            Ray ray_shadow(pi + L * EPS, L);
            for (const Primitive *pr_shadow : scene.primitives) {
                if (pr_shadow->light) continue;
                IntersectionResult res_intersect = pr_shadow->intersect(ray_shadow);
                if (res_intersect.hit != IntersectionResult::MISS && res_intersect.distance < dist_light) {
                    shade = 0;
                    break;
                }
            }

            // diffuse shading
            float k_diffuse = res.primitive->material.k_diffuse;
            if (k_diffuse > 0) {
                float dot = N.dot(L);
                if (dot > 0)
                    res.color += dot * k_diffuse * shade * res.primitive->material.color * light->material.color;
            }

            // specular shading
            float k_specular = 1.f - k_diffuse;
            if (k_specular > 0) {
                Vector3 V = ray.direction;
                Vector3 R = L - 2.f * L.dot(N) * N;
                float dot = V.dot(R);
                if (dot > 0)
                    res.color += powf(dot, 20) * k_specular * shade * light->material.color;
            }
        }

        // reflection
        float k_reflect = res.primitive->material.k_reflect;
        if (k_reflect > 0) {
            Vector3 R = ray.direction - 2.f * ray.direction.dot(N) * N;
            Ray ray_reflect(pi + R * EPS, R);
            RayTraceResult res_reflect = ray_trace(ray_reflect, remain_depth - 1);
            if (res_reflect.hit)
                res.color += k_reflect * res_reflect.color * res.primitive->material.color;
        }

        return res;
    }

    // TODO: camera position, screen position, z direction
    void render(uint8_t *out, int width, int height, int trace_depth) {
        float wx1 = -4, wx2 = 4, wy1 = 3, wy2 = -3;
        float dx = (wx2 - wx1) / width;
        float dy = (wy2 - wy1) / height;
        Vector3 o(0, 0, -5);
        for (int y = 0; y < height; ++y) {
            const float sy = wy1 + dy * y;
            for (int x = 0; x < width; ++x) {
                const float sx = wx1 + dx * x;
                Ray ray(o, Vector3(sx, sy, 0) - o);
                RayTraceResult res = ray_trace(ray, trace_depth);
                if (res.hit) {
                    const int idx = (y * width + x) * 3;
                    color_add_to_array(&out[idx], res.color);
                }
            }
        }
    }
};

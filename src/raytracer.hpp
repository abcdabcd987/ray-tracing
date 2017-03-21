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
        float distance;
        Color color;
        const Primitive *primitive;
    };

    RayTraceResult ray_trace(const Ray& ray, float refract_index, int remain_depth) const {
        RayTraceResult res = {.hit = false, .distance = 0, .color = Color(0, 0, 0), .primitive = nullptr};
        if (!remain_depth) return res;

        // find the nearest intersection
        float dist = std::numeric_limits<float>::max();
        IntersectionResult::HitType hit_type = IntersectionResult::MISS;
        for (const Primitive *pr : scene.primitives) {
            auto r = pr->intersect(ray);
            if (r.hit != IntersectionResult::MISS && r.distance < dist) {
                dist = r.distance;
                res.primitive = pr;
                hit_type = r.hit;
            }
        }

        // if no intersection
        if (!res.primitive) return res;

        // if light
        if (res.primitive->light) return {.hit = true, .distance = dist, .color = Color(1.f, 1.f, 1.f), .primitive = res.primitive};

        // if normal object
        res.hit = true;
        res.distance = dist;
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
                IntersectionResult r = pr_shadow->intersect(ray_shadow);
                if (r.hit != IntersectionResult::MISS && r.distance < dist_light) {
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
            float k_specular = res.primitive->material.k_specular;
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
            RayTraceResult r = ray_trace(ray_reflect, refract_index, remain_depth - 1);
            if (r.hit)
                res.color += k_reflect * r.color * res.primitive->material.color;
        }

        // refraction
        float k_refract = res.primitive->material.k_refract;
        if (k_refract > 0) {
            float k_refract_index = res.primitive->material.k_refract_index;
            float n = refract_index / k_refract_index;
            Vector3 Nd = hit_type == IntersectionResult::INSIDE ? -N : N;
            float cosI = -Nd.dot(ray.direction);
            float cosT2 = 1.f - n * n * (1.f - cosI * cosI);
            if (cosT2 > 0) {
                Vector3 T = n * ray.direction + (n * cosI - sqrtf(cosT2)) * N;
                Ray ray_refract(pi + T * EPS, T);
                RayTraceResult r = ray_trace(ray_refract, k_refract_index, remain_depth - 1);
                if (r.hit) {
                    Color absorb = res.primitive->material.color * 0.15f * -r.distance;
                    Color transparency = expf(absorb);
                    res.color += r.color * transparency;
                }
            }
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
                RayTraceResult res = ray_trace(ray, 1.f, trace_depth);
                if (res.hit) {
                    const int idx = (y * width + x) * 3;
                    color_add_to_array(&out[idx], res.color);
                }
            }
        }
    }
};

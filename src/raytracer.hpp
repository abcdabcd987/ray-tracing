#pragma once
#include <atomic>
#include <chrono>
#include <limits>
#include <vector>
#include <thread>
#include <cstdint>
#include <cassert>
#include "ctpl_stl.h"
#include "geometry.hpp"

struct RayTracer {
    struct TraceConfig {
        int num_trace_depth = 6;
        int num_box_light_sample = 256;
        int num_diffuse_reflect_sample = 128;
        int num_worker = 4;

        TraceConfig() {}
    };

    Scene scene;
    std::atomic<int> cnt_rendered;
    RayTracer(): scene() {}

    struct FindNearestResult {
        bool hit;
        float distance;
        const Primitive *primitive;
    };
    FindNearestResult find_nearest(const Ray &ray) const {
        FindNearestResult res = {.hit = false, .distance = std::numeric_limits<float>::max(), .primitive = nullptr};
        for (const Primitive *pr : scene.primitives) {
            auto r = pr->intersect(ray);
            if (r.hit != IntersectionResult::MISS && r.distance < res.distance) {
                res.hit = true;
                res.distance = r.distance;
                res.primitive = pr;
            }
        }
        return res;
    }

    struct CalcShadeResult {
        float shade;
        Vector3 light_direction;
    };
    float calc_shade_point_light(const Primitive *light, const Vector3 &light_diff, const Vector3& pi) const {
        Vector3 L = light_diff.normalized();
        Ray ray_shadow(pi + L * EPS, L);
        FindNearestResult r = find_nearest(ray_shadow);
        return r.primitive == light ? 1.f : .0f;
    }
    CalcShadeResult calc_shade(const Primitive *light, const Vector3 &pi, const TraceConfig &config) const {
        if (const Sphere *ls = dynamic_cast<const Sphere*>(light)) {
            Vector3 light_diff = ls->center - pi;
            float shade = calc_shade_point_light(ls, light_diff, pi);
            return {.shade = shade, .light_direction = light_diff.normalized()};
        } else if (const Box *lb = dynamic_cast<const Box*>(light)) {
            Vector3 L(0, 0, 0);
            float shade = .0;
            const int n = static_cast<int>(sqrt(config.num_box_light_sample));
            for (int i = 0, k = 0; i < n; ++i)
                for (int j = 0; j < n && k < config.num_box_light_sample; ++j, ++k) {
                    Vector3 ratio(randf() * (i+1)/n, randf(), randf() * (j+1)/n);
                    Vector3 light_point = lb->aabb.pos + ratio * lb->aabb.size;
                    Vector3 light_diff = light_point - pi;
                    L += light_diff.normalized();
                    shade += calc_shade_point_light(lb, light_diff, pi);
                }
            return {.shade = shade / config.num_box_light_sample, .light_direction = L / config.num_box_light_sample};
            // FIXME: This assumes y is small
//            for (int i = 0; i < NUM_BOX_LIGHT_SAMPLE; ++i) {
//                Vector3 ratio(randf(), randf(), randf());
//                Vector3 light_point = lb->aabb.pos + ratio * lb->aabb.size;
//                Vector3 light_diff = light_point - pi;
//                L += light_diff.normalized();
//                shade += calc_shade_point_light(lb, light_diff, pi);
//            }
//            return {.shade = shade / NUM_BOX_LIGHT_SAMPLE, .light_direction = L / NUM_BOX_LIGHT_SAMPLE};
        }
        return {.shade = 0};
    };

    struct RayTraceResult {
        bool hit;
        float distance;
        Color color;
        const Primitive *primitive;
    };
    RayTraceResult ray_trace(const Ray& ray, float refract_index, int depth, const TraceConfig &config) const {
        RayTraceResult res = {.hit = false, .distance = 0, .color = Color(0, 0, 0), .primitive = nullptr};
        if (depth > config.num_trace_depth) return res;

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
        for (const Primitive *light : scene.primitives) {
            // shadow
            if (!light->light) continue;
            CalcShadeResult res_shade = calc_shade(light, pi, config);
            Vector3 L = res_shade.light_direction;
            float shade = res_shade.shade;

            if (shade > 0) {
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
        }

        // reflection
        float k_reflect = res.primitive->material.k_reflect;
        if (k_reflect > 0) {
            float k_diffuse_reflect = res.primitive->material.k_diffuse_reflect;
            if (k_diffuse_reflect > 0 && depth < 2) {
                // diffuse reflection: only primary ray
                Vector3 RP = ray.direction - 2.f * ray.direction.dot(N) * N;
                Vector3 RN1 = Vector3(RP.z, RP.y, -RP.x);
                Vector3 RN2 = RP.cross(RN1);
                Color c(0, 0, 0);
                TraceConfig config_importance = config;
                config_importance.num_box_light_sample *= 0.25;
                for (int i = 0; i < config.num_diffuse_reflect_sample; ++i) {
                    float len = randf() * k_diffuse_reflect;
                    float angle = static_cast<float>(randf() * 2 * M_PI);
                    float xoff = len * cosf(angle), yoff = len * sinf(angle);
                    Vector3 R = (RP + RN1 * xoff + RN2 * yoff * k_diffuse_reflect).normalized();
                    Ray ray_reflect(pi + R * EPS, R);
                    RayTraceResult r = ray_trace(ray_reflect, refract_index, depth + 1, config_importance);
                    if (r.hit)
                        c += k_reflect * r.color * res.primitive->material.color;
                }
                res.color += c / config.num_diffuse_reflect_sample;
            } else {
                // perfect reflection
                Vector3 R = ray.direction - 2.f * ray.direction.dot(N) * N;
                Ray ray_reflect(pi + R * EPS, R);
                TraceConfig config_importance = config;
                config_importance.num_box_light_sample *= 0.5;
                RayTraceResult r = ray_trace(ray_reflect, refract_index, depth + 1, config_importance);
                if (r.hit)
                    res.color += k_reflect * r.color * res.primitive->material.color;
            }
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
                TraceConfig config_importance = config;
                config_importance.num_box_light_sample *= 0.5;
                RayTraceResult r = ray_trace(ray_refract, k_refract_index, depth + 1, config_importance);
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
    void render(uint8_t *out, int width, int height, const TraceConfig &config) {
        cnt_rendered.store(0);
        float wx1 = -4, wx2 = 4, wy1 = 3, wy2 = -3;
        float dx = (wx2 - wx1) / width;
        float dy = (wy2 - wy1) / height;
        Vector3 o(0, 0, -5);

        auto func = [&](int thread_id, int x, int y) {
            const float sy = wy1 + dy * y;
            const float sx = wx1 + dx * x;
            Ray ray(o, Vector3(sx, sy, 0) - o);
            RayTraceResult res = ray_trace(ray, 1.f, 0, config);
            if (res.hit) {
                const int idx = (y * width + x) * 3;
                color_add_to_array(&out[idx], res.color);
            }
            ++cnt_rendered;
        };

        fprintf(stderr, "start rendering using %d workers\n", config.num_worker);
        auto start = std::chrono::high_resolution_clock::now();
        ctpl::thread_pool pool(config.num_worker);
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                pool.push(func, x, y);
        const int total = width * height;
        for (;;) {
            int cnt = cnt_rendered.load();
            auto now = std::chrono::high_resolution_clock::now();
            auto sec = (now - start).count() / 1e9;
            fprintf(stderr, "\rrendered %d/%d pixels in %.3fs", cnt, total, sec);
            if (cnt == total) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        fprintf(stderr, "\ndone\n");
    }
};

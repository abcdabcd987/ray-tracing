#pragma once
#include <atomic>
#include <chrono>
#include <limits>
#include <vector>
#include <thread>
#include <tuple>
#include <cstdint>
#include <cassert>
#include "concurrentqueue.h"
#include "geometry.hpp"

struct RayTracer {
    struct TraceConfig {
        float num_light_sample_per_unit = 1.0f;
        int num_trace_depth = 3;
        int num_diffuse_reflect_sample = 32;
        int num_worker = 4;

        TraceConfig() {}
    };

    Scene scene;
    std::atomic<int> cnt_rendered;
    std::atomic<bool> flag_to_stop;
    std::atomic<bool> flag_stopped;
    RayTracer(): scene() {}

    FindNearestResult find_nearest(const Ray &ray) const {
        FindNearestResult res;
        // use kdtree:
        for (const Primitive *pr : scene.primitives)
            res.update(pr->intersect(ray), pr);
        for (Body *body : scene.bodies)
            res.update(body->kdtree.find_nearest(ray));
//        // use brute force:
//        for (const Primitive *pr : scene.primitives)
//            res.update(pr->intersect(ray), pr);
//        for (Body *body : scene.bodies)
//            for (const Triangle *t : body->triangles)
//                res.update(t->intersect(ray), t);
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
        if (light->type == Primitive::SPHERE) {
            const Sphere *ls = static_cast<const Sphere *>(light);
            Vector3 light_diff = ls->center - pi;
            float shade = calc_shade_point_light(ls, light_diff, pi);
            return {.shade = shade, .light_direction = light_diff.normalized()};
        } else if (light->type == Primitive::BOX) {
            const Box *lb = static_cast<const Box*>(light);
            Vector3 L(0, 0, 0);
            float shade = .0;
            const int n = lb->get_num_light_sample(config.num_light_sample_per_unit);
            for (int i = 0; i < n; ++i) {
                const Vector3 &light_point = lb->light_samples[i];
                Vector3 light_diff = light_point - pi;
                L += light_diff.normalized();
                shade += calc_shade_point_light(lb, light_diff, pi);
            }
            return {.shade = shade / n, .light_direction = L / n};
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
        FindNearestResult res_nearest = find_nearest(ray);
        if (res_nearest.hit == IntersectionResult::MISS) return res;
        res.hit = true;
        res.primitive = res_nearest.primitive;
        res.distance = res_nearest.distance;

        // if light
        if (res.primitive->light)
            return {.hit = true, .distance = res.distance, .color = res.primitive->material.color, .primitive = res.primitive};

        // if normal object
        Vector3 pi = ray.origin + ray.direction * res.distance; // intersection point
        Vector3 N = res.primitive->get_normal(pi);
        Color color_pi = res.primitive->get_color(pi);
        for (const Primitive *light : scene.lights) {
            // shadow
            CalcShadeResult res_shade = calc_shade(light, pi, config);
            Vector3 L = res_shade.light_direction;
            float shade = res_shade.shade;

            if (shade > 0) {
                // diffuse shading
                float k_diffuse = res.primitive->material.k_diffuse;
                if (k_diffuse > 0) {
                    float dot = N.dot(L);
                    if (dot > 0)
                        res.color += dot * k_diffuse * shade * color_pi * light->material.color;
                }

                // specular shading
                float k_specular = res.primitive->material.k_specular;
                if (k_specular > 0) {
                    Vector3 R = L - 2.f * L.dot(N) * N;
                    float dot = ray.direction.dot(R);
                    if (dot > 0)
                        res.color += powf(dot, 20) * k_specular * shade * light->material.color;
                }
            }
        }

        // reflection
        float k_reflect = res.primitive->material.k_reflect;
        if (k_reflect > 0) {
            float k_diffuse_reflect = res.primitive->material.k_diffuse_reflect;
            if (k_diffuse_reflect > 0 && depth <= 1) {
                // diffuse reflection: only primary ray
                Vector3 RP = ray.direction - 2.f * ray.direction.dot(N) * N;
                Vector3 RN1 = Vector3(RP.z, RP.y, -RP.x);
                Vector3 RN2 = RP.cross(RN1);
                Color c(0, 0, 0);
                TraceConfig config_importance = config;
                config_importance.num_light_sample_per_unit *= 0.25;
                for (int i = 0; i < config.num_diffuse_reflect_sample; ++i) {
//                    float len = randf() * k_diffuse_reflect;
//                    float angle = static_cast<float>(randf() * 2 * M_PI);
//                    float xoff = len * cosf(angle), yoff = len * sinf(angle);
//                    Vector3 R = (RP + RN1 * xoff + RN2 * yoff * k_diffuse_reflect).normalized();
                    Vector3 Nx, Nz, Ny = N;
                    if (fabsf(Ny.x) > fabs(Ny.y)) Nx = Vector3(Ny.z, 0, -Ny.x);
                    else Nx = Vector3(0, -Ny.z, Ny.y);
                    Nx = Nx.normalized();
                    Nz = Ny.cross(Nx).normalized();
                    Vector3 sample = uniform_sample_hemisphere();
                    Vector3 R = Vector3(
                            sample.x * Nx.x + sample.y * Ny.x + sample.z * Nz.x,
                            sample.x * Nx.y + sample.y * Ny.y + sample.z * Nz.y,
                            sample.x * Nx.z + sample.y * Ny.z + sample.z * Nz.z
                    );
                    Ray ray_reflect(pi + R * EPS, R);
                    RayTraceResult r = ray_trace(ray_reflect, refract_index, depth + 1, config_importance);
                    if (r.hit)
                        c += k_reflect * r.color * color_pi;
                }
                res.color += c / config.num_diffuse_reflect_sample;
            } else {
                // perfect reflection
                Vector3 R = ray.direction - 2.f * ray.direction.dot(N) * N;
                Ray ray_reflect(pi + R * EPS, R);
                TraceConfig config_importance = config;
                config_importance.num_light_sample_per_unit *= 0.5;
                RayTraceResult r = ray_trace(ray_reflect, refract_index, depth + 1, config_importance);
                if (r.hit)
                    res.color += k_reflect * r.color * color_pi;
            }
        }

        // refraction
        float k_refract = res.primitive->material.k_refract;
        if (k_refract > 0) {
            float k_refract_index = res.primitive->material.k_refract_index;
            float n = refract_index / k_refract_index;
            Vector3 Nd = res_nearest.hit == IntersectionResult::INSIDE ? -N : N;
            float cosI = -Nd.dot(ray.direction);
            float cosT2 = 1.f - n * n * (1.f - cosI * cosI);
            if (cosT2 > 0) {
                Vector3 T = n * ray.direction + (n * cosI - sqrtf(cosT2)) * Nd;
                Ray ray_refract(pi + T * EPS, T);
                TraceConfig config_importance = config;
                config_importance.num_light_sample_per_unit *= 0.5;
                RayTraceResult r = ray_trace(ray_refract, k_refract_index, depth + 1, config_importance);
                if (r.hit) {
//                    Color absorb = color_pi * 0.15f * -r.distance;
//                    Color transparency = expf(absorb);
                    Color transparency(1, 1, 1);
                    res.color += r.color * transparency;
                }
            }
        }

        return res;
    }

    // TODO: camera position, scre  en position, z direction
    bool render(uint8_t *out, int width, int height, const TraceConfig &config) {
        flag_to_stop = false;
        flag_stopped = false;
        cnt_rendered = 0;

        for (Primitive *light : scene.lights)
            light->sample_light(config.num_light_sample_per_unit);

        float wx1 = -4, wx2 = 4, wy1 = 3, wy2 = -3;
        float dx = (wx2 - wx1) / width;
        float dy = (wy2 - wy1) / height;
        Vector3 o(0, 0, -6);

        moodycamel::ConcurrentQueue<std::pair<int, int>> q;
        auto func = [&] {
            for (std::pair<int, int> item; q.try_dequeue(item);) {
                const int x = item.first, y = item.second;
                const float sy = wy1 + dy * y;
                const float sx = wx1 + dx * x;
                Ray ray(o, Vector3(sx, sy, -2) - o);
                RayTraceResult res = ray_trace(ray, 1.f, 1, config);
                Color color = res.hit ? res.color : Color(0, 0, 0);
                const int idx = (y * width + x) * 3;
                color_save_to_array(&out[idx], res.color);
                ++cnt_rendered;
            }
        };

        auto start = std::chrono::high_resolution_clock::now();
        std::vector<std::pair<int, int>> xys;
//        xys.emplace_back(157, 435);
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                xys.emplace_back(x, y);
        std::random_shuffle(xys.begin(), xys.end());
        for (const auto &xy : xys) q.enqueue(xy);

        std::vector<std::thread> workers;
        for (int i = 0; i < config.num_worker; ++i) workers.emplace_back(func);
        const int total = width * height;
        for (;;) {
            int cnt = cnt_rendered.load();
            auto now = std::chrono::high_resolution_clock::now();
            auto sec = (now - start).count() / 1e9;
            fprintf(stderr, "\rrendered %d/%d pixels using %d workers in %.3fs...", cnt, total, config.num_worker, sec);
            if (cnt == total) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));

            // if force stop
            if (flag_to_stop) {
                fprintf(stderr, "got stop flag..."); fflush(stderr);
                std::pair<int, int> ignore;
                while (q.try_dequeue(ignore));
                for (auto &worker : workers) worker.join();
                fprintf(stderr, "stopped\n");
                flag_stopped = true;
                return false;
            }
        }
        for (auto &worker : workers) worker.join();
        fprintf(stderr, "done\n");
        flag_stopped = true;
        return true;
    }

    void stop() {
        flag_to_stop = true;
    }
};

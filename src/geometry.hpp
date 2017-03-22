#pragma once

#include <cmath>
#include <vector>

constexpr float EPS = 1e-4;

struct Vector3 {
    union {
        struct { float x, y, z; };
        struct { float r, g, b; };
    };
    Vector3() : x(float(0)), y(float(0)), z(float(0)) {}
    Vector3(float xx) : x(xx), y(xx), z(xx) {}
    Vector3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    Vector3& operator+=(const Vector3 &v) { x += v.x, y += v.y, z += v.z; return *this; }
    Vector3& operator-=(const Vector3 &v) { x -= v.x, y -= v.y, z -= v.z; return *this; }
    Vector3& operator*=(const Vector3 &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
    Vector3& operator/=(const Vector3 &v) { x /= v.x, y /= v.y, z /= v.z; return *this; }
    Vector3& operator*=(float k) { x *= k; y *= k; z *= k; return *this; }
    Vector3& operator/=(float k) { x /= k; y /= k; z /= k; return *this; }
    friend Vector3 operator+(Vector3 u, const Vector3 &v) { return u += v; }
    friend Vector3 operator-(Vector3 u, const Vector3 &v) { return u -= v; }
    friend Vector3 operator*(Vector3 u, const Vector3 &v) { return u *= v; }
    friend Vector3 operator/(Vector3 u, const Vector3 &v) { return u /= v; }
    friend Vector3 operator*(Vector3 u, float k) { return u *= k; }
    friend Vector3 operator/(Vector3 u, float k) { return u /= k; }
    friend Vector3 expf(const Vector3& v) { return Vector3(std::expf(v.x), std::expf(v.y), std::expf(v.z)); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }
    float dot(const Vector3 &v) const { return x * v.x + y * v.y + z * v.z; }
    Vector3 cross(const Vector3 &v) const { return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    float length2() const { return dot(*this); }
    float length() const { return sqrtf(length2()); }
    Vector3 normalized() const {
        const float len = length();
        return Vector3(x / len, y / len, z / len);
    }
};
typedef Vector3 Color;


struct AABB {
    Vector3 pos;
    Vector3 size;

    AABB(const Vector3 &pos_, const Vector3& size_): pos(pos_), size(size_) {}

    bool intersect(const AABB& rhs) const {
        Vector3 v1 = rhs.pos, v2 = rhs.pos + rhs.size;
        Vector3 v3 = pos, v4 = pos + size;
        return ((v4.x > v1.x) && (v3.x < v2.x) &&
                (v4.y > v1.y) && (v3.y < v2.y) &&
                (v4.z > v1.z) && (v3.z < v2.z));
    }

    bool contain(const Vector3 &a_Pos) {
        Vector3 v1 = pos, v2 = pos + size;
        return ((a_Pos.x > (v1.x - EPS)) && (a_Pos.x < (v2.x + EPS)) &&
                (a_Pos.y > (v1.y - EPS)) && (a_Pos.y < (v2.y + EPS)) &&
                (a_Pos.z > (v1.z - EPS)) && (a_Pos.z < (v2.z + EPS)));
    }
};


struct Ray {
    Vector3 origin;
    Vector3 direction;
    Ray(const Vector3 &origin, const Vector3 &direction_) :
            origin(origin), direction(direction_.normalized()) {}
};


struct Material {
    Color color;
    float k_reflect;
    float k_diffuse;
    float k_diffuse_reflect;
    float k_specular;
    float k_refract;
    float k_refract_index;
    float k_ambient;
};


struct IntersectionResult {
    enum HitType {MISS, HIT, INSIDE} hit;
    float distance;
};


struct Primitive {
    Material material;
    float light;
    Primitive(): light(.0) {}
    virtual IntersectionResult intersect(const Ray& ray) const = 0;
    virtual Vector3 get_normal(const Vector3& pos) const = 0;
    virtual Color get_color(const Vector3 &pos) const { return material.color; }
};


struct Sphere : public Primitive {
    Vector3 center;
    float radius;
    Sphere(const Vector3 &center_, float radius_) :
            Primitive(), center(center_), radius(radius_) {}

    IntersectionResult intersect(const Ray &ray) const override {
        Vector3 v = ray.origin - center;
        float b = - v.dot(ray.direction);
        float det = b * b - v.length2() + radius * radius;
        if (det > 0) {
            det = sqrtf(det);
            float i1 = b - det, i2 = b + det;
            if (i2 > 0) {
                if (i1 < 0) return {.hit = IntersectionResult::INSIDE, .distance = i2};
                else return {.hit = IntersectionResult::HIT, .distance = i1};
            }
        }
        return {.hit = IntersectionResult::MISS};
    }

    Vector3 get_normal(const Vector3& pos) const override {
        return (pos - center).normalized();
    }
};


struct Plane : public Primitive {
    Vector3 normal;
    float distance;
    Plane(const Vector3& normal_, float distance_) :
            Primitive(), normal(normal_.normalized()), distance(distance_) {}

    IntersectionResult intersect(const Ray &ray) const override {
        float d = normal.dot(ray.direction);
        if (d != 0) {
            float dist = (normal.dot(ray.origin) + distance)  / -d;
            if (dist > 0) return {.hit = IntersectionResult::HIT, .distance = dist};
        }
        return {.hit = IntersectionResult::MISS};
    }

    Vector3 get_normal(const Vector3 &pos) const override {
        return normal;
    }
};


struct Box : public Primitive {
    AABB aabb;

    Box(const AABB &aabb_): Primitive(), aabb(aabb_) {}

    IntersectionResult intersect(const Ray &ray) const override {
        Vector3 d = ray.direction, o = ray.origin;
        Vector3 v1 = aabb.pos, v2 = aabb.pos + aabb.size;
        float dist[6] = {
                d.x ? (v1.x - o.x) / d.x : 0,
                d.x ? (v2.x - o.x) / d.x : 0,
                d.y ? (v1.y - o.y) / d.y : 0,
                d.y ? (v2.y - o.y) / d.y : 0,
                d.z ? (v1.z - o.z) / d.z : 0,
                d.z ? (v2.z - o.z) / d.z : 0,
        };
        IntersectionResult res = {.hit = IntersectionResult::MISS};
        for (int i = 0; i < 6; ++i) {
            if (!dist[i]) continue;
            Vector3 ip = o + dist[i] * d;
            if ((ip.x > (v1.x - EPS)) && (ip.x < (v2.x + EPS)) &&
                (ip.y > (v1.y - EPS)) && (ip.y < (v2.y + EPS)) &&
                (ip.z > (v1.z - EPS)) && (ip.z < (v2.z + EPS))) {
                if (res.hit == IntersectionResult::MISS || res.distance > dist[i])
                    res = {.hit = IntersectionResult::HIT, .distance = dist[i]};
            }
        }
        return res;
    }

    Vector3 get_normal(const Vector3 &pos) const override {
        assert(false);
        return Vector3(0, 0, 0);
    }
};


struct Scene {
    std::vector<Primitive*> primitives;
};


inline void color_add_to_array(uint8_t *out, const Color &color) {
    out[0] = static_cast<uint8_t>(std::min(out[0] + color.r * 255.f, 255.f));
    out[1] = static_cast<uint8_t>(std::min(out[1] + color.g * 255.f, 255.f));
    out[2] = static_cast<uint8_t>(std::min(out[2] + color.b * 255.f, 255.f));
}

inline float randf() {
    return rand() / static_cast<float>(RAND_MAX);
}

inline void save_ppm(const char *path, uint8_t *data, int width, int height) {
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

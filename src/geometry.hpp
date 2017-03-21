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
    Vector3& operator*=(float k) { x *= k; y *= k; z *= k; return *this; }
    friend Vector3 operator+(Vector3 u, const Vector3 &v) { return u += v; }
    friend Vector3 operator-(Vector3 u, const Vector3 &v) { return u -= v; }
    friend Vector3 operator*(Vector3 u, const Vector3 &v) { return u *= v; }
    friend Vector3 operator*(Vector3 u, float k) { return u *= k; }
    friend Vector3 expf(const Vector3& v) { return Vector3(std::expf(v.x), std::expf(v.y), std::expf(v.z)); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }
    float dot(const Vector3 &v) const { return x * v.x + y * v.y + z * v.z; }
    float length2() const { return dot(*this); }
    float length() const { return sqrtf(length2()); }
    Vector3 normalized() const {
        const float len = length();
        return Vector3(x / len, y / len, z / len);
    }
};
typedef Vector3 Color;


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
    float k_specular;
    float k_refract;
    float k_refract_index;
    float k_ambient;
};


struct IntersectionResult {
    const enum HitType {MISS, HIT, INSIDE} hit;
    const float distance;
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


struct Scene {
    std::vector<Primitive*> primitives;
};


inline void color_add_to_array(uint8_t *out, const Color &color) {
    out[0] = static_cast<uint8_t>(std::min(out[0] + color.r * 255.f, 255.f));
    out[1] = static_cast<uint8_t>(std::min(out[1] + color.g * 255.f, 255.f));
    out[2] = static_cast<uint8_t>(std::min(out[2] + color.b * 255.f, 255.f));
}

#pragma once

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <png.h>

constexpr float EPS = 1e-4;

struct Vector3 {
    union {
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
        float data[3];
    };

    Vector3() : x(0), y(0), z(0) {}

    Vector3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

    Vector3 &operator+=(const Vector3 &v) {
        x += v.x, y += v.y, z += v.z;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &v) {
        x -= v.x, y -= v.y, z -= v.z;
        return *this;
    }

    Vector3 &operator*=(const Vector3 &v) {
        x *= v.x, y *= v.y, z *= v.z;
        return *this;
    }

    Vector3 &operator/=(const Vector3 &v) {
        x /= v.x, y /= v.y, z /= v.z;
        return *this;
    }

    Vector3 &operator*=(float k) {
        x *= k;
        y *= k;
        z *= k;
        return *this;
    }

    Vector3 &operator/=(float k) {
        x /= k;
        y /= k;
        z /= k;
        return *this;
    }

    friend Vector3 operator+(Vector3 u, const Vector3 &v) { return u += v; }

    friend Vector3 operator-(Vector3 u, const Vector3 &v) { return u -= v; }

    friend Vector3 operator*(Vector3 u, const Vector3 &v) { return u *= v; }

    friend Vector3 operator/(Vector3 u, const Vector3 &v) { return u /= v; }

    friend Vector3 operator*(Vector3 u, float k) { return u *= k; }

    friend Vector3 operator*(float k, Vector3 u) { return u *= k; }

    friend Vector3 operator/(Vector3 u, float k) { return u /= k; }

    friend Vector3 expf(const Vector3 &v) { return Vector3(::expf(v.x), ::expf(v.y), ::expf(v.z)); }

    friend Vector3 min(const Vector3 &u, const Vector3 &v) {
        return Vector3(std::min(u.x, v.x), std::min(u.y, v.y), std::min(u.z, v.z));
    }

    friend Vector3 max(const Vector3 &u, const Vector3 &v) {
        return Vector3(std::max(u.x, v.x), std::max(u.y, v.y), std::max(u.z, v.z));
    }

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


struct Matrix3x3 {
    float m[3][3];

    Matrix3x3() : m() {}

    float operator()(int i, int j) const { return m[i][j]; }

    float &operator()(int i, int j) { return m[i][j]; }

    static Matrix3x3 scale(float k) {
        Matrix3x3 m;
        m(0, 0) = m(1, 1) = m(2, 2) = k;
        return m;
    }

    static Matrix3x3 construct_rotate(int a0, int a1, int a2, float theta) {
        float c = cosf(theta), s = sinf(theta);
        Matrix3x3 m;
        m(a0, a0) = 1;
        m(a1, a1) = c, m(a1, a2) = -s;
        m(a2, a1) = s, m(a2, a2) = c;
        return m;
    }

    static Matrix3x3 rotate_x(float theta) { return construct_rotate(0, 1, 2, theta); }

    static Matrix3x3 rotate_y(float theta) { return construct_rotate(0, 2, 1, theta); }

    static Matrix3x3 rotate_z(float theta) { return construct_rotate(2, 0, 1, theta); }

    friend Matrix3x3 operator*(const Matrix3x3 &a, const Matrix3x3 &b) {
        Matrix3x3 c;
        for (int k = 0; k < 3; ++k)
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    c(i, j) += a(i, k) * b(k, j);
        return c;
    }

    friend Vector3 operator*(const Matrix3x3 &m, const Vector3 &v) {
        return Vector3(
                m(0, 0) * v.data[0] + m(0, 1) * v.data[1] + m(0, 2) * v.data[2],
                m(1, 0) * v.data[0] + m(1, 1) * v.data[1] + m(1, 2) * v.data[2],
                m(2, 0) * v.data[0] + m(2, 1) * v.data[1] + m(2, 2) * v.data[2]
        );
    }
};


inline bool read_png_file(const char *filename, Color *(&out), int &width, int &height);

inline float randf() {
    return rand() / static_cast<float>(RAND_MAX);
}


struct Ray {
    Vector3 origin;
    Vector3 direction;

    Ray(const Vector3 &origin, const Vector3 &direction_) :
            origin(origin), direction(direction_.normalized()) {}
};


struct IntersectionResult {
    enum HitType {
        MISS, HIT, INSIDE
    } hit;
    float distance;
};


struct AABB {
    Vector3 pos;
    Vector3 size;

    AABB() : pos(), size() {}

    AABB(const Vector3 &pos_, const Vector3 &size_) : pos(pos_), size(size_) {}

    bool intersect(const AABB &rhs) const {
        Vector3 v1 = rhs.pos, v2 = rhs.pos + rhs.size;
        Vector3 v3 = pos, v4 = pos + size;
        return ((v4.x > v1.x) && (v3.x < v2.x) &&
                (v4.y > v1.y) && (v3.y < v2.y) &&
                (v4.z > v1.z) && (v3.z < v2.z));
    }

    IntersectionResult intersect(const Ray &ray) const {
        Vector3 d = ray.direction, o = ray.origin;
        Vector3 v1 = pos, v2 = pos + size;
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

    bool contain(const Vector3 &a_Pos) const {
        Vector3 v1 = pos, v2 = pos + size;
        return ((a_Pos.x > (v1.x - EPS)) && (a_Pos.x < (v2.x + EPS)) &&
                (a_Pos.y > (v1.y - EPS)) && (a_Pos.y < (v2.y + EPS)) &&
                (a_Pos.z > (v1.z - EPS)) && (a_Pos.z < (v2.z + EPS)));
    }

    void extend(const AABB &rhs) {
        if (size.data[0] == 0 && size.data[1] == 0 && size.data[2] == 0) {
            pos = rhs.pos;
            size = rhs.size;
        } else {
            Vector3 vmin = min(pos, rhs.pos);
            Vector3 vmax = max(pos + size, rhs.pos + rhs.size);
            pos = vmin;
            size = vmax - vmin;
        }
    }
};


struct Texture {
    virtual Color get_color(float u, float v) const = 0;

    virtual ~Texture() {}
};


struct GridTexture : public Texture {
    Color c0, c1;

    GridTexture(const Color &c0, const Color &c1) : c0(c0), c1(c1) {}

    Color get_color(float u, float v) const override {
        int a1 = (static_cast<int>(u) & 1) == 0;
        int a2 = (static_cast<int>(v) & 1) == 0;
        if (u < 0) a1 ^= 1;
        if (v < 0) a2 ^= 1;
        return (a1 ^ a2) ? c0 : c1;
    }
};


struct PNGTexture : public Texture {
    int width, height;
    Color *img;

    PNGTexture(const char *filename) : img(nullptr), width(0), height(0) {
        bool success = read_png_file(filename, img, width, height);
        if (!success)
            fprintf(stderr, "failed to load texture file: %s\n", filename);
    }

    Color get_color(float u, float v) const override {
        if (!img) return Color(1, 1, 1);
        // fetch a bilinearly filtered texel
        float fu = (u + 1000.0f) * width;
        float fv = (v + 1000.0f) * height;
        int u1 = ((int) fu) % width;
        int v1 = ((int) fv) % height;
        int u2 = (u1 + 1) % width;
        int v2 = (v1 + 1) % height;
        // calculate fractional parts of u and v
        float fracu = fu - floorf(fu);
        float fracv = fv - floorf(fv);
        // calculate weight factors
        float w1 = (1 - fracu) * (1 - fracv);
        float w2 = fracu * (1 - fracv);
        float w3 = (1 - fracu) * fracv;
        float w4 = fracu * fracv;
        // fetch four texels
        Color c1 = img[u1 + v1 * width];
        Color c2 = img[u2 + v1 * width];
        Color c3 = img[u1 + v2 * width];
        Color c4 = img[u2 + v2 * width];
        // scale and sum the four colors
        return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
    }

    ~PNGTexture() {
        free(img);
    }
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
    Texture *texture;
    float texture_uscale;
    float texture_vscale;
};


struct Primitive {
    enum Type {
        SPHERE, TRIANGLE, PLANE, BOX
    };
    Type type;
    bool light;
    Material material;
    Vector3 *light_samples;

    Primitive(Type type_) : type(type_), light(false), material(), light_samples(nullptr) {}

    virtual IntersectionResult intersect(const Ray &ray) const = 0;

    virtual Vector3 get_normal(const Vector3 &pos) const = 0;

    virtual Color get_color(const Vector3 &pos) const { return material.color; }

    virtual float get_volume() const { return 0; }

    virtual void sample_light(const float num_light_sample_per_unit) {}

    virtual int get_num_light_sample(float num_light_sample_per_unit) const {
        float volume = get_volume();
        return std::max(1, static_cast<int>(std::ceil(volume * num_light_sample_per_unit)));
    }

    int alloc_light_samples(float num_light_sample_per_unit) {
        int n = get_num_light_sample(num_light_sample_per_unit);
        delete[] light_samples;
        light_samples = static_cast<Vector3 *>(::operator new(n * sizeof(Vector3)));
        return n;
    }

    virtual ~Primitive() { delete[] light_samples; }
};


struct Sphere : public Primitive {
    Vector3 center;
    float radius;

    Sphere(const Vector3 &center_, float radius_) :
            Primitive(SPHERE), center(center_), radius(radius_) {}

    IntersectionResult intersect(const Ray &ray) const override {
        Vector3 v = ray.origin - center;
        float b = -v.dot(ray.direction);
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

    Vector3 get_normal(const Vector3 &pos) const override {
        return (pos - center).normalized();
    }

    float get_volume() const override {
        return 4.f / 3.f * static_cast<float>(M_PI) * radius * radius;
    }

    void sample_light(const float num_light_sample_per_unit) override {
        // see: http://stackoverflow.com/questions/5408276/sampling-uniformly-distributed-random-points-inside-a-spherical-volume
        int n = alloc_light_samples(num_light_sample_per_unit);
        for (int i = 0; i < n; ++i) {
            float phi = randf() * static_cast<float>(M_PI);
            float cos_theta = randf() * 2.f - 1.f;
            float u = randf();
            float theta = acosf(cos_theta);
            float r = radius * std::cbrtf(u);
            float x = r * sinf(theta) * cosf(phi);
            float y = r * sinf(theta) * sinf(phi);
            float z = r * cos_theta;
            light_samples[i] = Vector3(x, y, z);
        }
    }

    Color get_color(const Vector3 &pos) const override {
        if (!material.texture) return material.color;
        Vector3 vn = Vector3(0, 1, 0);
        Vector3 ve = Vector3(1, 0, 0);
        Vector3 vc = vn.cross(ve);
        Vector3 vp = (pos - center) / radius;
        float phi = acosf(-vp.dot(vn));
        float v = phi / static_cast<float>(M_PI);
        float theta = (acosf(ve.dot(vp) / sinf(phi))) * 2 / static_cast<float>(M_PI);
        float u = vc.dot(vp) >= 0 ? 1 - theta : theta;
        Color texture_color = material.texture->get_color(u * material.texture_uscale, v * material.texture_vscale);
        return texture_color * material.color;
    }
};


struct Triangle;

struct Vertex {
    Vector3 point;
    Vector3 normal;
    std::vector<Triangle *> neighbor;

    Vertex() : point(), normal(), neighbor() {}

    Vertex(float x, float y, float z) : point(x, y, z), normal(), neighbor() {}

    void calc_normal();
};


struct Triangle : public Primitive {
    Vertex *v0, *v1, *v2;
    Vector3 normal;

    Triangle(Vertex *v0_, Vertex *v1_, Vertex *v2_)
            : Primitive(TRIANGLE), v0(v0_), v1(v1_), v2(v2_),
              normal((v1->point - v0->point).cross(v2->point - v0->point).normalized()) {}

    void set_vertices(Vertex *v0_, Vertex *v1_, Vertex *v2_) {
        v0 = v0_;
        v1 = v1_;
        v2 = v2_;
        normal = (v1->point - v0->point).cross(v2->point - v0->point).normalized();
    }

    // see https://www.scratchapixel.com/code.php?id=11&origin=/lessons/3d-basic-rendering/ray-tracing-polygon-mesh
    bool calc_intersect(const Ray &ray, float &u, float &v, float &dist) const {
        IntersectionResult res = {.hit = IntersectionResult::MISS};
        Vector3 v0v1 = v1->point - v0->point;
        Vector3 v0v2 = v2->point - v0->point;
        Vector3 pvec = ray.direction.cross(v0v2);
        float det = v0v1.dot(pvec);

        // ray and triangle are parallel if det is close to 0
        if (fabs(det) < EPS) return false;

        float invDet = 1 / det;

        Vector3 tvec = ray.origin - v0->point;
        u = tvec.dot(pvec) * invDet;
        if (u < 0 || u > 1) return false;

        Vector3 qvec = tvec.cross(v0v1);
        v = ray.direction.dot(qvec) * invDet;
        if (v < 0 || u + v > 1) return false;

        dist = v0v2.dot(qvec) * invDet;
        return dist >= 0;
    }

    IntersectionResult intersect(const Ray &ray) const override {
        float u, v, dist;
        bool intersect = calc_intersect(ray, u, v, dist);
        if (!intersect) return {.hit = IntersectionResult::MISS};
        return {.hit = ray.direction.dot(normal) > 0 ? IntersectionResult::INSIDE : IntersectionResult::HIT,
                .distance = dist};
    }

    Vector3 get_normal(const Vector3 &pos) const override {
        float u, v, dist;
        calc_intersect(Ray(Vector3(0, 0, 0), pos), u, v, dist);
        Vector3 n = v0->normal * (1 - u - v) + v1->normal * u + v2->normal * v;
        return n; //.normalized();
    }

    AABB get_bounding_box() const {
        Vector3 vmin = min(v0->point, min(v1->point, v2->point));
        Vector3 vmax = max(v0->point, max(v1->point, v2->point));
        return AABB(vmin, vmax - vmin);
    }
};

inline void Vertex::calc_normal() {
    Vector3 n;
    for (Triangle *t : neighbor) n += t->normal;
    normal = n / neighbor.size();
}


struct Plane : public Primitive {
    Vector3 normal;
    float distance;

    Plane(const Vector3 &normal_, float distance_) :
            Primitive(Primitive::PLANE), normal(normal_.normalized()), distance(distance_) {}

    IntersectionResult intersect(const Ray &ray) const override {
        float d = normal.dot(ray.direction);
        if (d != 0) {
            float dist = (normal.dot(ray.origin) + distance) / -d;
            if (dist > 0) return {.hit = IntersectionResult::HIT, .distance = dist};
        }
        return {.hit = IntersectionResult::MISS};
    }

    Vector3 get_normal(const Vector3 &pos) const override {
        return normal;
    }

    Color get_color(const Vector3 &pos) const override {
        if (!material.texture) return material.color;
        Vector3 uaxis(normal.y, normal.z, -normal.x);
        Vector3 vaxis = uaxis.cross(normal);
        float u = pos.dot(uaxis) * material.texture_uscale;
        float v = pos.dot(vaxis) * material.texture_vscale;
        Color texture_color = material.texture->get_color(u, v);
        return texture_color * material.color;
    }
};


struct Box : public Primitive {
    AABB aabb;

    Box(const AABB &aabb_) : Primitive(Primitive::BOX), aabb(aabb_) {}

    IntersectionResult intersect(const Ray &ray) const override {
        return aabb.intersect(ray);
    }

    Vector3 get_normal(const Vector3 &pos) const override {
        assert(false);
        return Vector3(0, 0, 0);
    }

    float get_volume() const override {
        return aabb.size.x * aabb.size.y * aabb.size.z;
    }

    void sample_light(const float num_light_sample_per_unit) override {
        int n = alloc_light_samples(num_light_sample_per_unit);
        for (int i = 0; i < n; ++i) {
            Vector3 ratio = Vector3(randf(), randf(), randf());
            light_samples[i] = aabb.pos + ratio * aabb.size;
        }
    }
};


struct FindNearestResult {
    IntersectionResult::HitType hit = IntersectionResult::MISS;
    float distance = std::numeric_limits<float>::max();
    const Primitive *primitive = nullptr;

    void update(IntersectionResult::HitType rhs_hit, float rhs_distance, const Primitive *rhs_primitive) {
        if (rhs_hit != IntersectionResult::MISS &&
            (hit == IntersectionResult::MISS || distance > rhs_distance)) {
            hit = rhs_hit;
            distance = rhs_distance;
            primitive = rhs_primitive;
        }
    }

    void update(const IntersectionResult &rhs, const Primitive *rhs_primitive) {
        update(rhs.hit, rhs.distance, rhs_primitive);
    }

    void update(const FindNearestResult &rhs) {
        update(rhs.hit, rhs.distance, rhs.primitive);
    }
};


// ref: https://blog.frogslayer.com/kd-trees-for-faster-ray-tracing-with-triangles/
// ref: http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_7_Kd-Trees_and_More_Speed.shtml
// ref: https://github.com/ppwwyyxx/Ray-Tracing-Engine/blob/master/src/kdtree.cc
struct KDTree {
    struct Node {
        AABB bbox;
        Node *child[2];
        std::vector<const Triangle *> triangles;

        Node() : bbox(), child(), triangles() {}

        ~Node() {
            delete child[0];
            delete child[1];
        }
    };

    Node *root;
    static constexpr int NUM_LEAF_OBJS = 8;
    static constexpr int NUM_MAX_DEPTH = 32;

    KDTree() : root(nullptr) {}

    ~KDTree() { delete root; }

    void build(const std::vector<const Triangle *> &triangles) {
        delete root;
        root = build(triangles, 0);
    }

    FindNearestResult find_nearest(const Ray &ray) const {
        return find_nearest(ray, root, std::numeric_limits<float>::max());
    }

private:

    float get_split_plane_naive(const std::vector<const Triangle *> &triangles, int axis) const {
        float sum = 0;
        for (const Triangle *t : triangles) {
            sum += t->v0->point.data[axis];
            sum += t->v1->point.data[axis];
            sum += t->v2->point.data[axis];
        }
        return sum / (3 * triangles.size());
    }

    Node *build(const std::vector<const Triangle *> &triangles, int depth) const {
        Node *node = new Node();
        for (const Triangle *t : triangles)
            node->bbox.extend(t->get_bounding_box());
        if (triangles.size() >= NUM_LEAF_OBJS && depth < NUM_MAX_DEPTH) {
            int axis = depth % 3;
            float plane = get_split_plane_naive(triangles, axis);
            int common = 0;
            std::vector<const Triangle *> lef, rig;
            for (const Triangle *t : triangles) {
                bool in_lef = t->v0->point.data[axis] <= plane || t->v1->point.data[axis] <= plane ||
                              t->v2->point.data[axis] <= plane;
                bool in_rig = t->v0->point.data[axis] >= plane || t->v1->point.data[axis] >= plane ||
                              t->v2->point.data[axis] >= plane;
                if (in_lef) lef.emplace_back(t);
                if (in_rig) rig.emplace_back(t);
                if (in_lef && in_rig) ++common;
            }
            if (common * 2 < triangles.size()) {
                node->child[0] = build(lef, depth + 1);
                node->child[1] = build(rig, depth + 1);
                return node;
            }
        }

        // if too few triangles, or too deep, or too many common triangles
        node->triangles = triangles;
        return node;
    }

    FindNearestResult find_nearest(const Ray &ray, Node *node, float opt_dist) const {
        FindNearestResult res;
        IntersectionResult ibox = node->bbox.intersect(ray);
        if (ibox.hit == IntersectionResult::MISS) return res;
        if (ibox.distance > opt_dist) return res;
        if (node->child[0] || node->child[1]) {
            res.update(find_nearest(ray, node->child[0], opt_dist));
            opt_dist = std::min(opt_dist, res.distance);
            res.update(find_nearest(ray, node->child[1], opt_dist));
        } else {
            for (const Triangle *t : node->triangles)
                res.update(t->intersect(ray), t);
        }
        return res;
    }
};


struct Body {
    std::vector<Vector3> points;
    std::vector<Vertex *> vertices;
    std::vector<Triangle *> triangles;
    KDTree kdtree;
    Material material;
    Matrix3x3 w = Matrix3x3::scale(1.0f);
    Vector3 b;

    void set_material(const Material &m) {
        material = m;
        for (Triangle *t : triangles) t->material = m;
    }

    void scale(float k) {
        w = Matrix3x3::scale(k) * w;
        build();
    }

    void offset(const Vector3 &offset) {
        b += offset;
        build();
    }

    void rotate_xyz(float rad_x, float rad_y, float rad_z) {
        w = Matrix3x3::rotate_x(rad_x) * w;
        w = Matrix3x3::rotate_y(rad_y) * w;
        w = Matrix3x3::rotate_z(rad_z) * w;
        build();
    }

    void build() {
        for (size_t i = 0; i < points.size(); ++i)
            vertices[i]->point = w * points[i] + b;
        for (Triangle *t : triangles)
            t->set_vertices(t->v0, t->v1, t->v2);
        for (Vertex *v : vertices)
            v->calc_normal();
        std::vector<const Triangle *> v(triangles.begin(), triangles.end());
        kdtree.build(v);
    }

    ~Body() {
        for (Vertex *v : vertices) delete v;
        for (Triangle *t : triangles) delete t;
    }
};


struct Scene {
    std::vector<Primitive *> lights;
    std::vector<Primitive *> primitives;
    std::vector<Body *> bodies;

    void add(Primitive *p) {
        primitives.emplace_back(p);
        if (p->light) lights.emplace_back(p);
    }

    Body *load_obj(const char *path) {
        FILE *f = fopen(path, "r");
        if (!f) return nullptr;

        Body *body = new Body();
        char buf[100];
        while (fscanf(f, " %s", buf) != EOF) {
            if (strcmp(buf, "v") == 0) {
                float x, y, z;
                fscanf(f, "%f%f%f", &x, &y, &z);
                body->points.emplace_back(x, y, z);
                body->vertices.emplace_back(new Vertex(x, y, z));
            } else if (strcmp(buf, "f") == 0) {
                Vertex *v[3];
                for (int i = 0, idx; i < 3; ++i) {
                    fscanf(f, " %s", buf);
                    sscanf(buf, "%d", &idx);
                    v[i] = body->vertices[idx - 1];
                }
                Triangle *triangle = new Triangle(v[0], v[1], v[2]);
                for (int i = 0; i < 3; ++i)
                    v[i]->neighbor.emplace_back(triangle);
                body->triangles.emplace_back(triangle);
            } else if (buf[0] == '#'
                       || strcmp(buf, "mtllib") == 0
                       || strcmp(buf, "vn") == 0
                       || strcmp(buf, "vt") == 0
                       || strcmp(buf, "s") == 0
                       || strcmp(buf, "g") == 0
                       || strcmp(buf, "o") == 0
                       || strcmp(buf, "usemtl") == 0) {
                // not supported
                while (fgetc(f) != '\n');
            } else {
                // unexpected
                delete body;
                return nullptr;
            }
        }
        body->build();
        bodies.emplace_back(body);

        fclose(f);
        return body;
    }

    ~Scene() {
        for (Primitive *p : primitives) delete p;
        for (Body *b : bodies) delete b;
    }
};


inline void color_save_to_array(uint8_t *out, const Color &color) {
    out[0] = static_cast<uint8_t>(std::min(color.r * 255.f, 255.f));
    out[1] = static_cast<uint8_t>(std::min(color.g * 255.f, 255.f));
    out[2] = static_cast<uint8_t>(std::min(color.b * 255.f, 255.f));
}

inline void save_ppm(const char *path, uint8_t *data, int width, int height) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "failed to save ppm file to: %s\n", path);
        return;
    }
    fprintf(f, "P3\n");
    fprintf(f, "%d %d\n", width, height);
    fprintf(f, "255\n");
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = (y * width + x) * 3;
            fprintf(f, "%u\t%u\t%u\t", data[i], data[i + 1], data[i + 2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

inline Vector3 uniform_sample_hemisphere() {
    // cos(theta) = u1 = y
    // cos^2(theta) + sin^2(theta) = 1 -> sin(theta) = srtf(1 - cos^2(theta))
    float r1 = randf();
    float r2 = randf();
    float sinTheta = sqrtf(1 - r1 * r1);
    float phi = 2 * static_cast<float>(M_PI) * r2;
    float x = sinTheta * cosf(phi);
    float z = sinTheta * sinf(phi);
    return Vector3(x, r1, z);
}

// ref: https://gist.github.com/niw/5963798
inline bool read_png_file(const char *filename, Color *(&out), int &width, int &height) {
    // read png
    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return false;
    png_infop info = png_create_info_struct(png);
    if (!info) return false;
    if (setjmp(png_jmpbuf(png))) return false;
    png_init_io(png, fp);
    png_read_info(png, info);
    width = static_cast<int>(png_get_image_width(png, info));
    height = static_cast<int>(png_get_image_height(png, info));
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    if (bit_depth == 16) png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);
    png_read_update_info(png, info);
    png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++)
        row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(png, info));
    png_read_image(png, row_pointers);

    // save to Color array
    out = static_cast<Color *>(malloc(sizeof(Color) * width * height));
    for (int y = 0, k = 0; y < height; ++y) {
        png_byte *row = row_pointers[y];
        for (int x = 0; x < width; ++x, ++k) {
            png_byte *rgba = &row[x * 4];
            out[k].r = rgba[0] / 255.0f;
            out[k].g = rgba[1] / 255.0f;
            out[k].b = rgba[2] / 255.0f;
        }
    }

    // clean up
    for (int y = 0; y < height; ++y)
        free(row_pointers[y]);
    free(row_pointers);
    fclose(fp);
    return true;
}

// ref: https://gist.github.com/niw/5963798
inline bool _save_png(const char *path, uint8_t *data, int width, int height) {
    FILE *fp = fopen(path, "w");
    if (!fp) return false;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return false;
    png_infop info = png_create_info_struct(png);
    if (!info) return false;
    if (setjmp(png_jmpbuf(png))) return false;
    png_init_io(png, fp);
    png_set_IHDR(png, info, (png_uint_32) width, (png_uint_32) height, 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(png, info));
        png_byte *row = row_pointers[y];
        for (int x = 0; x < width; ++x) {
            png_byte *dst = &row[x * 4];
            uint8_t *src = &data[(y * width + x) * 3];
            dst[0] = src[0], dst[1] = src[1], dst[2] = src[2];
            dst[3] = 255;
        }
    }
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    fclose(fp);
    return true;
}

inline void save_png(const char *path, uint8_t *data, int width, int height) {
    if (!_save_png(path, data, width, height))
        fprintf(stderr, "failed to save png file to: %s\n", path);
}

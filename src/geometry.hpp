#pragma once
#include <cmath>
#include <cstdio>
#include <vector>

constexpr float EPS = 1e-4;

struct Vector3 {
    union {
        struct { float x, y, z; };
        struct { float r, g, b; };
        float data[3];
    };
    Vector3() : x(0), y(0), z(0) {}
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
    friend Vector3 operator*(float k, Vector3 u) { return u *= k; }
    friend Vector3 operator/(Vector3 u, float k) { return u /= k; }
    friend Vector3 expf(const Vector3& v) { return Vector3(std::expf(v.x), std::expf(v.y), std::expf(v.z)); }
    friend Vector3 min(const Vector3& u, const Vector3 &v) {
        return Vector3(std::min(u.x, v.x), std::min(u.y, v.y), std::min(u.z, v.z));
    }
    friend Vector3 max(const Vector3& u, const Vector3 &v) {
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
    enum HitType {MISS, HIT, INSIDE} hit;
    float distance;
};


struct AABB {
    Vector3 pos;
    Vector3 size;

    AABB(): pos(), size() {}
    AABB(const Vector3 &pos_, const Vector3& size_): pos(pos_), size(size_) {}

    bool intersect(const AABB& rhs) const {
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


struct Primitive {
    enum Type { SPHERE, TRIANGLE, PLANE, BOX };
    Type type;
    bool light;
    Material material;
    Vector3 *light_samples;
    Primitive(Type type_): type(type_), light(false), material(), light_samples(nullptr) {}
    virtual IntersectionResult intersect(const Ray& ray) const = 0;
    virtual Vector3 get_normal(const Vector3& pos) const = 0;
    virtual Color get_color(const Vector3 &pos) const { return material.color; }
    virtual float get_volume() const { return 0; }
    virtual void sample_light(const float num_light_sample_per_unit) { }
    virtual int get_num_light_sample(float num_light_sample_per_unit) const {
        float volume = get_volume();
        return std::max(1, static_cast<int>(std::ceil(volume * num_light_sample_per_unit)));
    }
    int alloc_light_samples(float num_light_sample_per_unit) {
        int n = get_num_light_sample(num_light_sample_per_unit);
        delete [] light_samples;
        light_samples = static_cast<Vector3*>(::operator new(n * sizeof(Vector3)));
        return n;
    }
    virtual ~Primitive() { delete [] light_samples; }
};


struct Sphere : public Primitive {
    Vector3 center;
    float radius;
    Sphere(const Vector3 &center_, float radius_) :
            Primitive(SPHERE), center(center_), radius(radius_) {}

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

    float get_volume() const override {
        return 4.f / 3.f * static_cast<float>(M_PI) * radius * radius;
    }

    void sample_light(const float num_light_sample_per_unit) override {
        // see: http://stackoverflow.com/questions/5408276/sampling-uniformly-distributed-random-points-inside-a-spherical-volume
        int n = alloc_light_samples(num_light_sample_per_unit);
        for (int i = 0; i < n; ++i) {
            float phi = randf() * static_cast<float>(M_PI);
            float cos_theta =  randf() * 2.f - 1.f;
            float u = randf();
            float theta = std::acos(cos_theta);
            float r = radius * std::cbrtf(u);
            float x = r * std::sinf(theta) * std::cosf(phi);
            float y = r * std::sinf(theta) * std::sinf(phi);
            float z = r * cos_theta;
            light_samples[i] = Vector3(x, y, z);
        }
    }
};


struct Triangle : public Primitive {
    Vector3 v0, v1, v2;
    Vector3 normal;

    Triangle(const Vector3 &v0_, const Vector3 &v1_, const Vector3 &v2_)
            : Primitive(TRIANGLE), v0(v0_), v1(v1_), v2(v2_),
              normal((v1-v0).cross(v2-v0).normalized())
    { }

    void set_vertices(const Vector3 &v0_, const Vector3 &v1_, const Vector3 &v2_) {
        v0 = v0_;
        v1 = v1_;
        v2 = v2_;
        normal = (v1-v0).cross(v2-v0).normalized();
    }

    IntersectionResult intersect(const Ray &ray) const override {
        // see https://www.scratchapixel.com/code.php?id=11&origin=/lessons/3d-basic-rendering/ray-tracing-polygon-mesh
        IntersectionResult res = {.hit = IntersectionResult::MISS};
        Vector3 v0v1 = v1 - v0;
        Vector3 v0v2 = v2 - v0;
        Vector3 pvec = ray.direction.cross(v0v2);
        float det = v0v1.dot(pvec);

        // ray and triangle are parallel if det is close to 0
        if (fabs(det) < EPS) return res;

        float invDet = 1 / det;

        Vector3 tvec = ray.origin - v0;
        float u = tvec.dot(pvec) * invDet;
        if (u < 0 || u > 1) return res;

        Vector3 qvec = tvec.cross(v0v1);
        float v = ray.direction.dot(qvec) * invDet;
        if (v < 0 || u + v > 1) return res;

        float dist = v0v2.dot(qvec) * invDet;
        if (dist < 0) return res;
        res.hit = ray.direction.dot(normal) > 0 ? IntersectionResult::INSIDE : IntersectionResult::HIT;
        res.distance = dist;
        return res;
//        if (dist < 0) return {.hit = IntersectionResult::INSIDE, .distance = dist};
//        return {.hit = IntersectionResult::HIT, .distance = dist};
    }

    Vector3 get_normal(const Vector3 &pos) const override {
        return normal;
    }

    AABB get_bounding_box() const {
        Vector3 vmin = min(v0, min(v1, v2));
        Vector3 vmax = max(v0, max(v1, v2));
        return AABB(vmin, vmax - vmin);
    }

    float get_volume() const override {
        // pretend that a triangle has a volume
        float area = .5f * (v2-v0).cross(v1-v0).length();
        return area * 0.1f;
    }

    void sample_light(const float num_light_sample_per_unit) override {
        int n = alloc_light_samples(num_light_sample_per_unit);
        for (int i = 0; i < n; ++i) {
            // see: http://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle
            Vector3 v0v1 = v1 - v0;
            Vector3 v0v2 = v2 - v0;
            float sqrt_r1 = std::sqrtf(randf()), r2 = randf();
            light_samples[i] = sqrt_r1 * (1.f - r2) * v0v1 + r2 * sqrt_r1 * v0v2;
        }
    }
};


struct Plane : public Primitive {
    Vector3 normal;
    float distance;
    Plane(const Vector3& normal_, float distance_) :
            Primitive(Primitive::PLANE), normal(normal_.normalized()), distance(distance_) {}

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

    Box(const AABB &aabb_): Primitive(Primitive::BOX), aabb(aabb_) {}

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
    void update(const IntersectionResult& rhs, const Primitive *rhs_primitive) {
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
        Node* child[2];
        std::vector<const Triangle*> triangles;
        Node(): bbox(), child(), triangles() {}
        ~Node() { delete child[0]; delete child[1]; }
    };
    Node *root;
    static constexpr int NUM_LEAF_OBJS = 8;
    static constexpr int NUM_MAX_DEPTH = 32;
    KDTree(): root(nullptr) {}
    ~KDTree() { delete root; }

    void build(const std::vector<const Triangle*> &triangles) {
        delete root;
        root = build(triangles, 0);
    }

    FindNearestResult find_nearest(const Ray &ray) const {
        return find_nearest(ray, root, std::numeric_limits<float>::max());
    }

private:

    float get_split_plane_naive(const std::vector<const Triangle*> &triangles, int axis) const {
        float sum = 0;
        for (const Triangle *t : triangles) {
            sum += t->v0.data[axis];
            sum += t->v1.data[axis];
            sum += t->v2.data[axis];
        }
        return sum / (3 * triangles.size());
    }

    Node *build(const std::vector<const Triangle*> &triangles, int depth) const {
        Node *node = new Node();
        for (const Triangle *t : triangles)
            node->bbox.extend(t->get_bounding_box());
        if (triangles.size() >= NUM_LEAF_OBJS && depth < NUM_MAX_DEPTH) {
            int axis = depth % 3;
            float plane = get_split_plane_naive(triangles, axis);
            int common = 0;
            std::vector<const Triangle*> lef, rig;
            for (const Triangle *t : triangles) {
                bool in_lef = t->v0.data[axis] <= plane || t->v1.data[axis] <= plane || t->v2.data[axis] <= plane;
                bool in_rig = t->v0.data[axis] >= plane || t->v1.data[axis] >= plane || t->v2.data[axis] >= plane;
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
    std::vector<Triangle*> triangles;
    KDTree kdtree;
    Body(): triangles(), kdtree() {}
    void set_material(const Material &m) { for (Triangle* t : triangles) t->material = m; }

    void scale(float k) {
        for (Triangle* t : triangles) {
            t->v0 *= k;
            t->v1 *= k;
            t->v2 *= k;
        }
    }

    void offset(const Vector3 &offset) {
        for (Triangle* t : triangles) {
            t->v0 += offset;
            t->v1 += offset;
            t->v2 += offset;
        }
    }

    void build() {
        std::vector<const Triangle*> v(triangles.begin(), triangles.end());
        kdtree.build(v);
    }
};


struct Scene {
    std::vector<Primitive*> all_primitives;
    std::vector<Primitive*> lights;
    std::vector<Primitive*> primitives;
    std::vector<Body*> bodies;

    void build() {
        for (Body *body : bodies)
            body->build();
    }

    void add(Primitive *p) {
        all_primitives.emplace_back(p);
        if (p->type != Primitive::TRIANGLE) primitives.emplace_back(p);
        if (p->light) lights.emplace_back(p);
    }

    Body* load_obj(const char *path) {
        FILE *f = fopen(path, "r");
        if (!f) return nullptr;

        std::vector<Vector3> vertices;
        Body *body = new Body();
        char buf[100];
        while (fscanf(f, " %s", buf) != EOF) {
            if (strcmp(buf, "v") == 0) {
                float x, y, z;
                fscanf(f, "%f%f%f", &x, &y, &z);
                vertices.emplace_back(x, y, z);
            } else if (strcmp(buf, "f") == 0) {
                int v[3];
                for (int i = 0; i < 3; ++i) {
                    fscanf(f, " %s", buf);
                    sscanf(buf, "%d", &v[i]);
                    --v[i];
                }
                Triangle *triangle = new Triangle(vertices[v[0]], vertices[v[1]], vertices[v[2]]);
                body->triangles.emplace_back(triangle);
                add(triangle);
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
                return nullptr;
            }
        }
        bodies.emplace_back(body);

        fclose(f);
        return body;
    }

    ~Scene() {
        for (Primitive *p : all_primitives) delete p;
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

#pragma once
#include "raytracer.hpp"

void add_scene2(RayTracer &tracer) {
    GridTexture *texture_grid = new GridTexture(Color(0, 0, 0), Color(1, 1, 1));
    PNGTexture *texture_ground = new PNGTexture("../resources/ground.png");
    PNGTexture *texture_watermelon = new PNGTexture("../resources/watermelon.png");

    Material plane_material = {
            .color = Color(1, 1, 1),
            .k_reflect = 0.0f,
            .k_diffuse = 1.0f,
            .k_diffuse_reflect = 0.0f,
            .k_specular = .8f,
            .k_refract = 0,
            .k_refract_index = 1.f,
            .texture = nullptr,
            .texture_uscale = 1,
            .texture_vscale = 1,
    };
    Material def_material = {
            .color = Color(1, 1, 1),
            .k_reflect = 0,
            .k_diffuse = 0,
            .k_diffuse_reflect = 0,
            .k_specular = 0,
            .k_refract = 0,
            .k_refract_index = 1,
            .texture = nullptr,
            .texture_uscale = 1,
            .texture_vscale = 1,
    };

    Material m;
    Primitive *p;
    Body *body;

    p = new Box(AABB(Vector3(-0.5f, 2.36f, -2.43f), Vector3(1.0f, 0.1f, 1.0f)));
    p->light = true;
    p->material = def_material;
    p->material.color = Color(1, 1, 1);
    tracer.scene.add(p);

    p = new Plane(Vector3(0, 1, 0), 6);
    p->material = plane_material;
    p->material.texture = texture_ground;
    p->material.texture_uscale = 0.1f;
    p->material.texture_vscale = 0.1f;
    tracer.scene.add(p);

    p = new Plane(Vector3(0, 0, -1), 20);
    p->material = plane_material;
    p->material.texture = texture_grid;
    tracer.scene.add(p);

    body = Body::load_obj("../resources/Arma.obj");
    body->scale(1.5f);
    body->offset(Vector3(-2.0f, -2.0f, 0.0f));
    m = def_material;
    m.color = Color(0.8, 0.6, 0.6);
    m.k_diffuse = 1;
    m.k_reflect = 0.5;
    body->set_material(m);
    tracer.scene.add(body);

    body = Body::load_obj("../resources/Buddha.obj");
    body->scale(1.5f);
    body->offset(Vector3(0.0f, -2.0f, 0.0f));
    m = def_material;
    m.color = Color(0.6, 0.8, 0.6);
    m.k_diffuse = 1;
    m.k_reflect = 0.5;
    body->set_material(m);
    tracer.scene.add(body);

    body = Body::load_obj("../resources/bunny.fine.obj");
    body->scale(1.5f);
    body->offset(Vector3(2.0f, -2.0f, 0.0f));
    m = def_material;
    m.color = Color(0.6, 0.6, 0.8);
    m.k_diffuse = 1;
    m.k_reflect = 0.5;
    body->set_material(m);
    tracer.scene.add(body);
}

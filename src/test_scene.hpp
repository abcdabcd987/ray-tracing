#pragma once
#include "raytracer.hpp"

void add_scene2(RayTracer &tracer) {
    Material plane_material = {
            .color = Color(.4f, .3f, .3f),
            .k_reflect = 0.2f,
            .k_diffuse = 1.0f,
            .k_diffuse_reflect = 0.0f,
            .k_specular = .2f,
            .k_refract = 0,
            .k_refract_index = 1.f,
            .k_ambient = 0,
            .texture = nullptr,
            .texture_uscale = 1,
            .texture_vscale = 1,
    };
    Material light_material = {
            .color = Color(1, 1, 1),
            .k_reflect = 0.f,
            .k_diffuse = 1.f,
            .k_diffuse_reflect = 0.f,
            .k_specular = 0.f,
            .k_refract = 1.f,
            .k_refract_index = 1.f,
            .k_ambient = 0,
            .texture = nullptr,
            .texture_uscale = 1,
            .texture_vscale = 1,
    };
    Material def_material = {
            .color = Color(.2f, .2f, .2f),
            .k_reflect = 0,
            .k_diffuse = 0.2f,
            .k_diffuse_reflect = 0,
            .k_specular = .8f,
            .k_refract = 0,
            .k_refract_index = 1.5f,
            .k_ambient = 0,
            .texture = nullptr,
            .texture_uscale = 1,
            .texture_vscale = 1,
    };

    Material m;
    Primitive *p;
    Body *body;
    GridTexture *texture_grid = new GridTexture(Color(0, 0, 0), Color(1, 1, 1));
    PNGTexture *texture_ground = new PNGTexture("../resources/ground.png");

    p = new Box(AABB(Vector3(-0.5f, 2.36f, -2.43f), Vector3(1.0f, 0.1f, 1.0f)));
    p->light = true;
    p->material = light_material;
    p->material.color = Color(1, 1, 1);
    tracer.scene.add(p);

//    p = new Sphere(Vector3(2, 2, -3), .1f);
//    p->light = true;
//    p->material = light_material;
//    p->material.color = Color(1, 1, 1);
//    tracer.scene.add(p);

    p = new Plane(Vector3(0, 1, 0), 6);
    p->material = plane_material;
    p->material.color = Color(.4f, .3f, .3f);
    p->material.texture = texture_ground;
    p->material.texture_uscale = 0.1f;
    p->material.texture_vscale = 0.1f;
    tracer.scene.add(p);

    p = new Plane(Vector3(0, -1, 0), 6);
    p->material = plane_material;
    p->material.color = Color(.6f, .0f, .0f);
    tracer.scene.add(p);

    p = new Plane(Vector3(-1, 0, 0), 6);
    p->material = plane_material;
    p->material.color = Color(.0f, .6f, .0f);
    tracer.scene.add(p);

    p = new Plane(Vector3(1, 0, 0), 6);
    p->material = plane_material;
    p->material.color = Color(.0f, .0f, .6f);
    tracer.scene.add(p);

    p = new Plane(Vector3(0, 0, -1), 6);
    p->material = plane_material;
    p->material.color = Color(.7f, .7f, .3f);
    p->material.texture = texture_grid;
    tracer.scene.add(p);

    p = new Sphere(Vector3(-1.370f, -0.630f, 3.350f), 1.597f);
    p->material = def_material;
    p->material.k_reflect = 0.0f;
    p->material.k_refract = 0.0f;
    p->material.k_refract_index = 1.0f;
    p->material.k_diffuse = 1.0f;
    p->material.color = Color(1, 1, 1);
    p->material.k_diffuse_reflect = 0.0f;
    p->material.texture = texture_grid;
    p->material.texture_uscale = 10;
    p->material.texture_vscale = 10;
    tracer.scene.add(p);

//    p = new Sphere(Vector3(-1.370f, -0.630f, 3.350f), 1.597f);
//    p->material = def_material;
//    p->material.k_reflect = 0.0f;
//    p->material.k_refract = 1.0f;
//    p->material.k_refract_index = 0.613f;
//    p->material.k_diffuse = 0.0f;
//    p->material.color = Color(0.7f, 0.7f, 1.0f);
//    p->material.k_diffuse_reflect = 0.0f;
//    tracer.scene.add(p);

//    p = new Sphere(Vector3(2.5f, 1, 2), 1);
//    p->material = def_material;
//    p->material.k_reflect = 0.5f;
//    p->material.k_refract = 0.0f;
//    p->material.k_refract_index = 1.3f;
//    p->material.k_diffuse = 0.1f;
//    p->material.color = Color(0.7f, 0.7f, 1.0f);
//    tracer.scene.add(p);
    m = def_material;
    m.k_reflect = 1.0f;
    m.k_refract = 0.0f;
    m.k_diffuse = 0.0f;
    m.color = Color(0.7f, 0.7f, 1.0f);
//    body = tracer.scene.load_obj("../resources/sphere.obj");
//    body->set_material(m);
//    body->offset(Vector3(2.5f, 1.0f, 2.0f));
//    body = tracer.scene.load_obj("../resources/teapot.obj");
//    body->set_material(m);
//    body->scale(0.5f);
//    body->offset(Vector3(-2.0f, 0.5f, 1.0f));

//    m = def_material;
//    m.k_reflect = 0.1f;
//    m.k_refract = 1.0f;
//    m.k_refract_index = 1.2f;
//    m.color = Color(0.7f, 0.7f, 1.0f);
//    body = tracer.scene.load_obj("../resources/sphere.obj");
//    body->set_material(m);
//    body->scale(0.8);
//    body->offset(Vector3(-1.5f, -1.5f, 0));
//    body = tracer.scene.load_obj("../resources/teapot.obj");
//    body->set_material(m);
//    body->scale(0.4);
//    body->offset(Vector3(-2.5f, -2.5f, -0.5f));
    p = new Sphere(Vector3(-2, -2, 0), 1.0f);
    p->material = def_material;
    p->material.k_reflect = 0.2f;
    p->material.k_refract = 0.8f;
    p->material.k_refract_index = 1.2f;
    p->material.color = Color(0.7f, 0.7f, 1.0f);
    tracer.scene.add(p);
//    for (int x = -4; x <= 4; x++)
//        for (int y = -4; y <= 4; y++) {
//            Sphere *s = new Sphere(Vector3(-3.0f+x*0.4f, -3.0f+y*0.4f, 3.0f), 0.15f);
//            s->material = def_material;
//            s->material.k_reflect = 0;
//            s->material.k_refract = 0;
//            s->material.k_specular = 0.6f;
//            s->material.k_diffuse = 0.6f;
//            s->material.color = Color(0.3f, 1.0f, 0.4f);
//            tracer.scene.add(s);
//        }

    Material teapot_material = {
            .color = Color(1, 1, 1),
            .k_reflect = 0.2f,
            .k_diffuse = 0.5f,
            .k_diffuse_reflect = 0.0f,
            .k_specular = 0.5f,
            .k_refract = 0.0f,
            .k_refract_index = 1.f,
            .k_ambient = 0,
    };
//    body = tracer.scene.load_obj("../resources/teapot.obj");
//    body->set_material(teapot_material);
//    body->scale(0.4);
//    body->offset(Vector3(2, -3, 1));

    Material sphere_material = {
            .color = Color(.7f, .8f, .9f),
            .k_reflect = 0.0f,
            .k_diffuse = 0.9f,
            .k_diffuse_reflect = 0.0f,
            .k_specular = 0.1f,
            .k_refract = 0.0f,
            .k_refract_index = 1.f,
            .k_ambient = 0,
    };
//    body = tracer.scene.load_obj("../resources/sphere.obj");
//    body->set_material(sphere_material);
//    body->scale(0.7);
//    body->offset(Vector3(1, 2.f, 0));
}

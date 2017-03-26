#pragma once
#include "raytracer.hpp"

// Don't care about the memory leak

void add_scene1(RayTracer &tracer) {
    Material def_material = {
            .color = Color(.2f, .2f, .2f),
            .k_specular = .8f,
            .k_ambient = 0,
            .k_refract_index = 1.5f,
            .k_refract = 0,
            .k_diffuse = 0.2f,
            .k_reflect = 0,
            .k_diffuse_reflect = 0,
    };

    // ground plane
    Primitive *p0 = new Plane(Vector3(0, 1, 0), 4.4f);
    p0->material = def_material;
    p0->material.k_reflect = 0.0f;
    p0->material.k_refract = 0.0f;
    p0->material.k_diffuse = 1.0f;
    p0->material.color = Color(0.4f, 0.3f, 0.3f);
    // big sphere
    Primitive *p1 = new Sphere(Vector3(0, -0.8f, 7), 2);
    p1->material = def_material;
    p1->material.k_reflect = 0.2f;
    p1->material.k_refract = 0.8f;
    p1->material.k_refract_index = 1.3f;
    p1->material.color = Color(0.7f, 0.7f, 1.0f);
    p1->material.k_diffuse_reflect = 0.3f;
    // small sphere
    Primitive *p2 = new Sphere(Vector3(-5, -0.8f, 7), 2);
    p2->material = def_material;
    p2->material.k_reflect = 0.5f;
    p2->material.k_refract = 0.0f;
    p2->material.k_refract_index = 1.3f;
    p2->material.k_diffuse = 0.1f;
    p2->material.color = Color(0.7f, 0.7f, 1.0f);
    p2->material.k_diffuse_reflect = 0.6f;
    // third sphere
    Primitive *p3 = new Sphere(Vector3(5, -0.8f, 7), 2);
    p3->material = def_material;
    p3->material.k_reflect = 0.5f;
    p3->material.k_refract = 0.0f;
    p3->material.k_refract_index = 1.3f;
    p3->material.k_diffuse = 0.1f;
    p3->material.color = Color(0.7f, 0.7f, 1.0f);
#if 1
    // area light
    Primitive *p4 = new Box(AABB(Vector3(-1, 5, 4), Vector3( 2, 0.1f, 2)));
    p4->light = true;
    p4->material = def_material;
    p4->material.color = Color(1, 1, 1);
#else
    // light source 1
    Primitive *p4 = new Sphere(Vector3(0, 5, 5), 0.1f);
    p4->light = true;
    p4->material = def_material;
    p4->material.color = Color(1, 1, 1);
#endif
    // back plane
    Primitive *p5 = new Plane(Vector3(0, 0, -1), 12.4f);
    p5->material = def_material;
    p5->material.k_reflect = 0.0f;
    p5->material.k_refract = 0.0f;
    p5->material.k_diffuse = 1.0f;
    p5->material.color = Color(0.4f, 0.3f, 0.3f);
    // ceiling
    Primitive *p6 = new Plane(Vector3(0, -1, 0), 5.2f);
    p6->material = def_material;
    p6->material.k_reflect = 0.0f;
    p6->material.k_refract = 0.0f;
    p6->material.k_diffuse = 1.0f;
    p6->material.color = Color(0.4f, 0.3f, 0.3f);


    tracer.scene.add(p0);
    tracer.scene.add(p1);
    tracer.scene.add(p2);
    tracer.scene.add(p3);
    tracer.scene.add(p4);
    tracer.scene.add(p5);
    tracer.scene.add(p6);
}

void add_scene2(RayTracer &tracer) {
    Material sphere_material = {
            .color = Color(.7f, .8f, .9f),
            .k_specular = 0.1f,
            .k_ambient = 0,
            .k_refract_index = 1.f,
            .k_refract = 0.05f,
            .k_diffuse = 0.9f,
            .k_reflect = 0.2f,
            .k_diffuse_reflect = 0.3f,
    };
    Material plane_material = {
            .color = Color(.4f, .3f, .3f),
            .k_specular = .8f,
            .k_ambient = 0,
            .k_refract_index = 1.f,
            .k_refract = 0,
            .k_diffuse = 1.f,
            .k_reflect = 0,
            .k_diffuse_reflect = 0.f,
    };
    Material light_material = {
            .color = Color(1, 1, 1),
            .k_specular = 0.f,
            .k_ambient = 0,
            .k_refract_index = 1.f,
            .k_refract = 1.f,
            .k_diffuse = 1.f,
            .k_reflect = 0.f,
            .k_diffuse_reflect = 0.f,
    };

    Primitive *p;

//    for (int i = -1; i <= 1; ++i)
//        for (int j = -1; j <= 1; ++j) {
//            p = new Sphere(Vector3(0.2f * i, 2.f, -3.f + 0.2f * j), .1f);
//            p->light = true;
//            p->material = light_material;
//            p->material.color = Color(1, 1, 1) / 9.f;
//            tracer.scene.add(p);
//        }
//    p = new Box(AABB(Vector3(-1, 2, -2), Vector3(2, 0.1f, 2)));
//    p->light = true;
//    p->material = light_material;
//    p->material.color = Color(1, 1, 1);
//    tracer.scene.add(p);
    p = new Sphere(Vector3(-2, 2, -3), .1f);
    p->light = true;
    p->material = light_material;
    p->material.color = Color(1, 1, 1);
    tracer.scene.add(p);
    p = new Sphere(Vector3(2, 2, -3), .1f);
    p->light = true;
    p->material = light_material;
    p->material.color = Color(1, 1, 1);
    tracer.scene.add(p);

    p = new Plane(Vector3(0, 1, 0), 4.4f);
    p->material = plane_material;
    p->material.color = Color(.4f, .3f, .3f);
    tracer.scene.add(p);

    p = new Plane(Vector3(0, -1, 0), 4.4f);
    p->material = plane_material;
    p->material.color = Color(.6f, .0f, .0f);
    tracer.scene.add(p);

    p = new Plane(Vector3(-1, 0, 0), 4.4f);
    p->material = plane_material;
    p->material.color = Color(.0f, .6f, .0f);
    tracer.scene.add(p);

    p = new Plane(Vector3(1, 0, 0), 4.4f);
    p->material = plane_material;
    p->material.color = Color(.0f, .0f, .6f);
    tracer.scene.add(p);

    p = new Plane(Vector3(0, 0, -1), 4.4f);
    p->material = plane_material;
    p->material.color = Color(.7f, .7f, .3f);
    tracer.scene.add(p);

    Body *body = tracer.scene.load_obj("../models/teapot.obj");
    body->set_material(sphere_material);
}

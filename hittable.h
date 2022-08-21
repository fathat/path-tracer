#pragma once
#include "types.h"

class ray;
class material;

struct hit_record {
    point3 p;
    vec3_d normal;
    double t;

    shared_ptr<material> mat;

    bool front_face;

    void set_face_normal(const ray& r, const vec3_d& outward_normal);
};

class hittable {
    public:
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};
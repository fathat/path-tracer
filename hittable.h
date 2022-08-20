#pragma once
#include "types.h"

class ray;

struct hit_record {
    point3 p;
    vec3_d normal;
    double t;
};

class hittable {
    public:
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};
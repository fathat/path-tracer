#pragma once
#include "aabb.h"
#include "types.h"

class ray_t;
class material_t;

struct hit_record_t {
    point3 p;
    dvec3_t normal;
    double t;

    // texture coords
    dvec2_t uv;

    shared_ptr<material_t> mat;

    bool front_face;

    void set_face_normal(const ray_t& r, const dvec3_t& outward_normal);
};

/**
 * \brief Abstract class for any object which can be hit by a ray. 
 */
class hittable_t {
    public:
        virtual bool hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const = 0;
        virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const = 0;
};
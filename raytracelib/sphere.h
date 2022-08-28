#pragma once

#include "hittable.h"

class sphere_t : public hittable_t {
public:
    sphere_t() = delete;
    sphere_t(point3 cen, double r, const shared_ptr<material_t>& material) : center(cen), radius(r), mat(material) {};

    virtual bool hit(
        const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

    point3 center;
    double radius;
    shared_ptr<material_t> mat;
};
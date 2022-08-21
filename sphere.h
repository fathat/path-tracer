#pragma once

#include "hittable.h"

class sphere : public hittable {
public:
    sphere() = delete;
    sphere(point3 cen, double r, const shared_ptr<material>& material) : center(cen), radius(r), mat(material) {};

    virtual bool hit(
        const ray& r, double t_min, double t_max, hit_record& rec) const override;

    point3 center;
    double radius;
    shared_ptr<material> mat;
};
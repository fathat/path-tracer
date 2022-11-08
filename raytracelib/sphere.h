#pragma once

#include "hittable.h"

class sphere_t : public hittable_t {
public:
    sphere_t() = delete;
    sphere_t(point3 cen, double r, const shared_ptr<material_t>& material) : center0(cen), center1(cen), radius(r), mat(material) {};
    sphere_t(point3 cen, point3 cen1, double t0, double t1, double r, const shared_ptr<material_t>& material)
        : center0(cen), center1(cen1), time0(t0), time1(t1), radius(r), mat(material) {}
    

    virtual bool hit(
        const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

    point3 center() const { return center0; }

    point3 center(double time) const {
        if(!moving()) {
            return center();
        }
        return center0 + ((time - time0) / (time1 - time0))*(center1 - center0);
    }

    bool moving() const {
        return glm::epsilonNotEqual(time0, time1, 0.0001);
    }

    point3 center0;
    point3 center1;
    double radius;
    shared_ptr<material_t> mat;
    double time0=0, time1=0;
};
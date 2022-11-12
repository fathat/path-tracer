#pragma once
#include <vector>
#include "types.h"
#include "hittable.h"

class rect_t : public hittable_t {
public:
    rect_t() = delete;

    rect_t(double w, double h, dvec3_t l, glm::quat r,
        shared_ptr<material_t> mat)
        : width(w), height(h), mp(mat), location(l), rotation(r) {};

    virtual bool hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const override;

    dmat4_t transform() const;

public:
    shared_ptr<material_t> mp;
    double width, height;
    dvec3_t location;
    glm::quat rotation; 
};

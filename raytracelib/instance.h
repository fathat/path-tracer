#pragma once
#include "hittable.h"

class instance_t : public hittable_t {
public:
    instance_t(shared_ptr<hittable_t> source, const dmat4_t& transform);

    bool hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;
    bool bounding_box(double time0, double time1, aabb_t& output_box) const override;

protected:
    shared_ptr<hittable_t> m_source;
    dmat4_t m_transform;
    dmat4_t m_inverse;
    aabb_t m_aabb;
};

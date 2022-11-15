#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "rect.h"

class box_t : public hittable_t {
public:

    box_t(dvec3_t center, dquat rotation, double width, double height, double depth, const shared_ptr<material_t>& mat);

    bool hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;
    bool bounding_box(double time0, double time1, aabb_t& output_box) const override;

protected:

    std::vector<dvec3_t> m_vertices;
    aabb_t m_cached_bb;

    dmat4_t m_cached_transform;
    dmat4_t m_cached_inverse_transform;

    dvec3_t m_center;
    dquat m_rotation;
    double m_width, m_height, m_depth;
    shared_ptr<rect_t> m_front, m_back, m_left, m_right, m_top, m_bottom;
    hittable_list_t m_sides;
    shared_ptr<material_t> m_material;
};
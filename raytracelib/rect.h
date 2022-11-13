#pragma once
#include <vector>
#include "types.h"
#include "hittable.h"

class rect_t : public hittable_t {
public:
    rect_t() = delete;

    rect_t(double w, double h, dvec3_t center, glm::quat rotation, shared_ptr<material_t> mat);

    virtual bool hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const override;

    dmat4_t transform() const { return m_cached_transform; }
    dmat4_t inverse_transform() const { return m_cached_inverse_transform; }

protected:

    void calc_transform();
    void calc_bounding_box();

    dmat4_t m_cached_transform;
    dmat4_t m_cached_inverse_transform;
    aabb_t m_cached_bb;

    shared_ptr<material_t> m_material;
    double m_width, m_height;
    dvec3_t m_center;
    glm::quat m_rotation; 
};



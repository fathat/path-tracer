#pragma once

#include "hittable.h"

class sphere_t : public hittable_t {
public:
    sphere_t() = delete;
    sphere_t(point3 cen, double r, const shared_ptr<material_t>& material) : m_center0(cen), m_center1(cen), m_radius(r), m_mat(material) {};
    sphere_t(point3 cen, point3 cen1, double t0, double t1, double r, const shared_ptr<material_t>& material)
        : m_center0(cen), m_center1(cen1), m_time0(t0), m_time1(t1), m_radius(r), m_mat(material) {}
    

    virtual bool hit(
        const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const override;

    [[nodiscard]] point3 center() const { return m_center0; }

    [[nodiscard]] point3 center(double time) const {
        if(!moving()) {
            return center();
        }
        return m_center0 + ((time - m_time0) / (m_time1 - m_time0))*(m_center1 - m_center0);
    }

    [[nodiscard]] bool moving() const {
        return glm::epsilonNotEqual(m_time0, m_time1, 0.0001);
    }

    [[nodiscard]] double radius() const { return m_radius; }

protected:
    point3 m_center0;
    point3 m_center1;
    double m_radius;
    shared_ptr<material_t> m_mat;
    double m_time0=0, m_time1=0;
};

extern dvec2_t get_sphere_uv(const point3& p);
#pragma once

#include "types.h"
#include "ray.h"

class aabb_t {
public:
    aabb_t() {}
    aabb_t(const point3& a, const point3& b) { m_minimum = a; m_maximum = b; }

    [[nodiscard]] point3 min() const { return m_minimum; }
    [[nodiscard]] point3 max() const { return m_maximum; }

    void expand(const aabb_t& other);

    [[nodiscard]] bool hit(const ray_t& r, double t_min, double t_max) const;
protected:
    point3 m_minimum;
    point3 m_maximum;
};

inline aabb_t surrounding_box(aabb_t box0, aabb_t box1) {

    point3 small(fmin(box0.min().x, box1.min().x),
        fmin(box0.min().y, box1.min().y),
        fmin(box0.min().z, box1.min().z));

    point3 big(fmax(box0.max().x, box1.max().x),
        fmax(box0.max().y, box1.max().y),
        fmax(box0.max().z, box1.max().z));

    return aabb_t(small, big);
}
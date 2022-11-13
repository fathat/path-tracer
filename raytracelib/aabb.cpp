#include "aabb.h"

void aabb_t::expand(const aabb_t& other) {
    m_minimum.x = std::min(m_minimum.x, other.min().x);
    m_minimum.y = std::min(m_minimum.y, other.min().y);
    m_minimum.z = std::min(m_minimum.z, other.min().z);

    m_maximum.x = std::max(m_maximum.x, other.max().x);
    m_maximum.y = std::max(m_maximum.y, other.max().y);
    m_maximum.z = std::max(m_maximum.z, other.max().z);
}

bool aabb_t::hit(const ray_t& r, double t_min, double t_max) const {
    for (int a = 0; a < 3; a++) {
        auto invD = 1.0 / r.direction()[a];
        auto t0 = (min()[a] - r.origin()[a]) * invD;
        auto t1 = (max()[a] - r.origin()[a]) * invD;
        if (invD < 0.0)
            std::swap(t0, t1);
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min)
            return false;
    }
    return true;
}

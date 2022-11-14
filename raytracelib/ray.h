#pragma once

#include "types.h"

class ray_t {
public:

    ray_t(): m_orig(0,0,0), m_dir(0, 0, 1) {}

    ray_t(const dvec3_t& origin, const dvec3_t& direction, double time=0)
        : m_orig(origin), m_dir(direction), m_time(time) {}

    [[nodiscard]] dvec3_t origin() const { return m_orig; }
    [[nodiscard]] dvec3_t direction() const { return m_dir; }
    [[nodiscard]] double time() const { return m_time; }

    [[nodiscard]] dvec3_t at(const double t) const {
        return m_orig + t*m_dir;
    }

    ray_t transformed(const glm::mat4x4& transform) const {
        auto origin = transform * glm::vec4(m_orig.x, m_orig.y, m_orig.z, 1.0f);
        auto dir = transform * glm::vec4(m_dir.x, m_dir.y, m_dir.z, 0.0f);
        return {origin, dir, m_time};
    }

    ray_t transformed(dvec3_t location, glm::quat rotation) const {
        auto new_origin = location + m_orig;
        auto new_direction4 = rotation * glm::vec4{m_dir.x, m_dir.y, m_dir.z, 0.0};
        dvec3_t new_direction = dvec3_t{new_direction4.x, new_direction4.y, new_direction4.z};
        return ray_t{new_origin, new_direction, m_time};
    }

protected:
    dvec3_t m_orig;
    dvec3_t m_dir;
    double m_time;
};
#pragma once

#include "types.h"

class ray_t {
public:

    ray_t(): m_orig(0,0,0), m_dir(0, 0, 1) {}

    ray_t(const dvec3_t& origin, const dvec3_t& direction)
        : m_orig(origin), m_dir(direction) {}

    [[nodiscard]] dvec3_t origin() const { return m_orig; }
    [[nodiscard]] dvec3_t direction() const { return m_dir; }

    [[nodiscard]] dvec3_t at(const double t) const {
        return m_orig + t*m_dir;
    }

protected:
    dvec3_t m_orig;
    dvec3_t m_dir;
};
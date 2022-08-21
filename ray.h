#pragma once

#include "types.h"

class ray {
public:

    ray(): m_orig(0,0,0), m_dir(0, 0, 1) {}

    ray(const vec3_d& origin, const vec3_d& direction)
        : m_orig(origin), m_dir(direction) {}

    [[nodiscard]] vec3_d origin() const { return m_orig; }
    [[nodiscard]] vec3_d direction() const { return m_dir; }

    [[nodiscard]] vec3_d at(const double t) const {
        return m_orig + t*m_dir;
    }

protected:
    vec3_d m_orig;
    vec3_d m_dir;
};
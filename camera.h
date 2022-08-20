#pragma once
#include "types.h"
#include "ray.h"

class camera {
    public:
        camera(const int w, const int h);

        [[nodiscard]] ray get_ray(double u, double v) const {
            return ray(m_origin, m_lower_left_corner + u*m_horizontal + v*m_vertical - m_origin);
        }

    private:
        point3 m_origin;
        point3 m_lower_left_corner;
        vec3_d m_horizontal;
        vec3_d m_vertical;
};
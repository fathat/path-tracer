#pragma once
#include "types.h"
#include "ray.h"

class camera {
public:
    camera(const int width, const int height, double vfov, point3 look_from, point3 look_at, vec3_d up);

    [[nodiscard]] ray get_ray(double s, double t) const;

private:
    point3 m_origin;
    point3 m_lower_left_corner;
    vec3_d m_horizontal;
    vec3_d m_vertical;
    double m_vfov;
};
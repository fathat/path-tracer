#pragma once
#include "types.h"
#include "ray.h"

class camera {
public:
    camera(const int width, const int height, double vfov, point3 look_from, point3 look_at, vec3_d up, double aperture, double focus_dist);

    void update(const int width, const int height, double vfov, point3 look_from, point3 look_at, vec3_d up, double aperture, double focus_dist);

    [[nodiscard]] ray get_ray(double s, double t) const;

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }

    void resize(int width, int height);

private:
    int m_width;
    int m_height;

    point3 m_origin;
    point3 m_look_at;
    vec3_d m_vup;
    double m_aperture;
    double m_focus_dist;

    point3 m_lower_left_corner;
    vec3_d m_horizontal;
    vec3_d m_vertical;
    double m_vfov;

    vec3_d m_u, m_v, m_w;
    double m_lens_radius;
};
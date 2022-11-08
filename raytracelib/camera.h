#pragma once
#include "types.h"
#include "ray.h"

class camera_t {
public:
    camera_t(
        const int width, 
        const int height, 
        double vfov, 
        point3 look_from, 
        point3 look_at, 
        dvec3_t up, 
        double aperture, 
        double focus_dist,
        double time0 = 0,
        double time1 = 0);

    void update(const int width, const int height, double vfov, point3 look_from, point3 look_at, dvec3_t up, double aperture, double focus_dist, double time0, double time1);

    [[nodiscard]] ray_t get_ray(double s, double t) const;

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }

    void resize(int width, int height);

private:
    int m_width;
    int m_height;

    point3 m_origin;
    point3 m_look_at;
    dvec3_t m_vup;
    double m_aperture;
    double m_focus_dist;

    point3 m_lower_left_corner;
    dvec3_t m_horizontal;
    dvec3_t m_vertical;
    double m_vfov;

    dvec3_t m_u, m_v, m_w;
    double m_lens_radius;

    double m_time0, m_time1;
};
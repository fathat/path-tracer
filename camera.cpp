#include "camera.h"

using glm::normalize;
using glm::cross;

camera::camera(const int width, const int height, double vfov, point3 look_from, point3 look_at, vec3_d up, double aperture, double focus_dist):
    m_width(width), m_height(height), m_vfov(vfov) {
    update(width, height, vfov, look_from, look_at, up, aperture, focus_dist);
}

void camera::update(const int width, const int height, double vfov, point3 look_from, point3 look_at, vec3_d up,
    double aperture, double focus_dist) {
    const auto theta = degrees_to_radians(m_vfov);
    auto htan = tan(theta/2);
    const auto aspect_ratio = static_cast<double>(width) / static_cast<double>(height);
    const auto viewport_height = 2.0 * htan;
    const auto viewport_width = aspect_ratio * viewport_height;

    m_look_at = look_at;
    m_vup = up;
    m_aperture = aperture;
    m_focus_dist = focus_dist;
    m_width = width;
    m_height = height;

    m_w = normalize(look_from - look_at);
    m_u = normalize(cross(up, m_w));
    m_v = cross(m_w, m_u);

    m_origin = look_from;
    m_horizontal = focus_dist * viewport_width * m_u;
    m_vertical = focus_dist * viewport_height * m_v;
    m_lower_left_corner = m_origin - m_horizontal/2.0 - m_vertical/2.0 - focus_dist*m_w;

    m_lens_radius = aperture / 2.0;    
}

ray camera::get_ray(double s, double t) const {
    //return ray(m_origin, m_lower_left_corner + s*m_horizontal + t*m_vertical - m_origin);
    const vec3_d rd = m_lens_radius * random_in_unit_disk();
    const vec3_d offset = m_u * rd.x + m_v * rd.y;

    return ray(
        m_origin + offset,
        m_lower_left_corner + s*m_horizontal + t*m_vertical - m_origin - offset
    );
}

void camera::resize(int width, int height) {
    m_width = width;
    m_height = height;
    update(width, height, m_vfov, m_origin, m_look_at, m_vup, m_aperture, m_focus_dist);
}

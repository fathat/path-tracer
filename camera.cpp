#include "camera.h"

using glm::normalize;

camera::camera(const int width, const int height, double vfov, point3 look_from, point3 look_at, vec3_d up): m_vfov(vfov) {
    const auto theta = degrees_to_radians(vfov);
    auto htan = tan(theta/2);
    const auto aspect_ratio = static_cast<double>(width) / static_cast<double>(height);
    const auto viewport_height = 2.0 * htan;
    const auto viewport_width = aspect_ratio * viewport_height;
    constexpr auto focal_length = 1.0;
        

    vec3_d w = normalize(look_from - look_at);
    vec3_d u = normalize(cross(up, w));
    auto v = cross(w, u);

    m_origin = look_from;
    m_horizontal = viewport_width * u;
    m_vertical = viewport_height * v;
    m_lower_left_corner = m_origin - m_horizontal/2.0 - m_vertical/2.0 - w;
}

ray camera::get_ray(double s, double t) const {
     return ray(m_origin, m_lower_left_corner + s*m_horizontal + t*m_vertical - m_origin);
}

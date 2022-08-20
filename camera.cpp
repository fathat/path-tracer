#include "camera.h"

camera::camera(const int w, const int h) {
    const auto aspect_ratio = static_cast<double>(w) / static_cast<double>(h);
    const auto viewport_height = 2.0;
    const auto viewport_width = aspect_ratio * viewport_height;
    const auto focal_length = 1.0;

    m_origin = point3(0, 0, 0);
    m_horizontal = vec3_d(viewport_width, 0.0, 0.0);
    m_vertical = vec3_d(0.0, viewport_height, 0.0);
    m_lower_left_corner = m_origin - m_horizontal/2.0 - m_vertical/2.0 - vec3_d(0, 0, focal_length);
}

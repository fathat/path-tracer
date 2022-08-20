#include "image_buffer.h"

constexpr int num_channels = 4;
using glm::clamp;

image_buffer::image_buffer(const uint32_t width, const uint32_t height): m_w(width), m_h(height) {
    m_buffer = std::make_unique<uint8_t[]>(m_w * m_h * num_channels);
}

void image_buffer::write(const uint32_t x, const uint32_t y, const color& color, int samples_per_pixel) {
    const size_t base_index = x * num_channels + y * m_w * num_channels;

    auto r = color.r;
    auto g = color.g;
    auto b = color.b;

    const auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    m_buffer[base_index] = static_cast<uint8_t>(color.a * 255);
    m_buffer[base_index + 1] = static_cast<uint8_t>(clamp(color.b, 0.0, 1.0) * 255);
    m_buffer[base_index + 2] = static_cast<uint8_t>(clamp(color.g, 0.0, 1.0) * 255);
    m_buffer[base_index + 3] = static_cast<uint8_t>(clamp(color.r, 0.0, 1.0) * 255);
}

int image_buffer::pitch() const { return m_w * num_channels; }

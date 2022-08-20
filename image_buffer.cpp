#include "image_buffer.h"

constexpr int num_channels = 4;

image_buffer::image_buffer(const uint32_t width, const uint32_t height): m_w(width), m_h(height) {
    m_buffer = std::make_unique<uint8_t[]>(m_w * m_h * num_channels);
}

void image_buffer::write(const uint32_t x, const uint32_t y, const color& color) {
    const size_t base_index = x * num_channels + y * m_w * num_channels;

    m_buffer[base_index] = static_cast<uint8_t>(color.a * 255);
    m_buffer[base_index + 1] = static_cast<uint8_t>(color.b * 255);
    m_buffer[base_index + 2] = static_cast<uint8_t>(color.g * 255);
    m_buffer[base_index + 3] = static_cast<uint8_t>(color.r * 255);
}

int image_buffer::pitch() const { return m_w * num_channels; }

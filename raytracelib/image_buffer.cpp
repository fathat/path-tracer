#include "image_buffer.h"

constexpr int num_channels = 4;
using glm::clamp;

image_buffer_t::image_buffer_t(const uint32_t width, const uint32_t height): m_w(width), m_h(height) {
    m_buffer = std::make_unique<uint8_t[]>(m_w * m_h * num_channels);
}

image_buffer_t::image_buffer_t(const image_buffer_t& src): m_w(src.width()), m_h(src.height()) {
    m_buffer = std::make_unique<uint8_t[]>(src.width() * src.height() * num_channels);
    memcpy(m_buffer.get(), src.data(), src.width() * src.height() * num_channels);
}

void image_buffer_t::write(const uint32_t x, const uint32_t y, const color_t& color, int samples_per_pixel) {
    const size_t base_index = x * num_channels + y * m_w * num_channels;

    auto r = color.r;
    auto g = color.g;
    auto b = color.b;

    const auto scale = 1.0 / static_cast<double>(samples_per_pixel);
    r = sqrt(r * scale);
    g = sqrt(g * scale);
    b = sqrt(b * scale);

    m_buffer[base_index] = static_cast<uint8_t>(color.a * 255);
    m_buffer[base_index + 1] = static_cast<uint8_t>(clamp(b, 0.0, 0.999) * 256);
    m_buffer[base_index + 2] = static_cast<uint8_t>(clamp(g, 0.0, 0.999) * 256);
    m_buffer[base_index + 3] = static_cast<uint8_t>(clamp(r, 0.0, 0.999) * 256);
}

void image_buffer_t::write_raw(const uint32_t x, const uint32_t y, const color_t& color) {
    const size_t base_index = x * num_channels + y * m_w * num_channels;
    auto r = color.r;
    auto g = color.g;
    auto b = color.b;

    m_buffer[base_index] = static_cast<uint8_t>(color.a * 255);
    m_buffer[base_index + 1] = static_cast<uint8_t>(clamp(b, 0.0, 0.999) * 256);
    m_buffer[base_index + 2] = static_cast<uint8_t>(clamp(g, 0.0, 0.999) * 256);
    m_buffer[base_index + 3] = static_cast<uint8_t>(clamp(r, 0.0, 0.999) * 256);
}

color_t image_buffer_t::read(const double x, const double y) const {
    const int ix = static_cast<int>(x * (m_w-1));
    const int iy = static_cast<int>(y * (m_h-1));

    const int base_index = ix * num_channels + iy * m_w * num_channels;
    const color_t c = {
        m_buffer[base_index+3] / 255.0,
        m_buffer[base_index+2] / 255.0,
        m_buffer[base_index+1] / 255.0,
        m_buffer[base_index] / 255.0,
    };
    return c;
}

#ifdef THREADS
void image_buffer_t::write_line_sync(const uint32_t y, const color_t data[], int samples_per_pixel) {
    std::lock_guard guard(m_mutex);

    for(int x = 0; x < m_w; x++) {
        write(x, y, data[x], samples_per_pixel);
    }
}

#endif

int image_buffer_t::pitch() const { return m_w * num_channels; }

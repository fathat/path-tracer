#pragma once
#include <memory>
#include "types.h"
#include "color.h"

#ifdef THREADS
#include <thread>
#include <mutex>
#endif


class image_buffer_t
{
public:
    image_buffer_t() = delete;

    image_buffer_t(const uint32_t width, const uint32_t height);
    image_buffer_t(const image_buffer_t& src);

    void write(const uint32_t x, const uint32_t y, const color_t& color, int samples_per_pixel=1);

    void write_raw(const uint32_t x, const uint32_t y, const color_t& color);
    color_t read(const double x, const double y) const;

#ifdef THREADS
    void write_line_sync(const uint32_t y, const color_t data[], int samples_per_pixel);
    std::mutex* mutex() { return &m_mutex; }
    
#endif

    [[nodiscard]] uint8_t* data() const { return m_buffer.get(); }
    [[nodiscard]] int pitch() const;
    [[nodiscard]] int width() const { return m_w; }
    [[nodiscard]] int height() const { return m_h; }
    

protected:
    int m_w;
    int m_h;

    std::unique_ptr<uint8_t[]> m_buffer;

#ifdef THREADS
    std::mutex m_mutex;
#endif

};



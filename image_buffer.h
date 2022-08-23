#pragma once
#include <memory>

#include "types.h"

#ifdef THREADS
#include <thread>
#include <mutex>
#endif


#include <SDL_render.h>

#include "color.h"

class image_buffer
{
public:
    image_buffer() = delete;

    image_buffer(const uint32_t width, const uint32_t height);

    void write(const uint32_t x, const uint32_t y, const color& color, int samples_per_pixel=1);

#ifdef THREADS
    void write_line_sync(const uint32_t y, const color data[], int samples_per_pixel);
    void update_texture_sync(SDL_Texture* texture);
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



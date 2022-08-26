#include "texture.h"

streaming_image_texture_t::streaming_image_texture_t(SDL_Renderer* renderer, int width, int height):
    m_width(width),
    m_height(height),
    m_buffer(std::make_unique<image_buffer_t>(width, height)),
    m_renderer(renderer) {

    m_texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        m_width, 
        m_height
    );

    int access;
    int actualW, actualH;
    SDL_QueryTexture(m_texture, &m_format, &access, &actualW, &actualH);

    // any graphics card should really support this
    assert(actualW == width);
    assert(actualH == height);
    assert(m_format == SDL_PIXELFORMAT_RGBA8888);
}

streaming_image_texture_t::~streaming_image_texture_t() {
    if(m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}

void streaming_image_texture_t::update_texture_sync() {
#ifdef THREADS
    std::lock_guard guard(*m_buffer->mutex());
#endif
    uint8_t* pixels;
    int pitch;
    // SDL_LockTexture is a bit faster than SDL_UpdateTexture for writes
    SDL_LockTexture(m_texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
    memcpy(pixels, m_buffer->data(), m_width*m_height*4);
    SDL_UnlockTexture(m_texture);
}

void streaming_image_texture_t::present() {
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
}

void streaming_image_texture_t::resize(int width, int height) {

    const image_buffer_t old_data(*m_buffer);

    m_width = width;
    m_height = height;
    SDL_DestroyTexture(m_texture);
    m_buffer = make_unique<image_buffer_t>(width, height);

    for(int iy = 0; iy < height; iy++) {
        for(int ix = 0; ix < width; ix ++) {
            double x = static_cast<double>(ix) / width;
            double y = static_cast<double>(iy) / height;
            m_buffer->write_raw(ix, iy, old_data.read(x, y));
        }    
    }
    
    m_texture = SDL_CreateTexture(
        m_renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        m_width, 
        m_height
    );
}

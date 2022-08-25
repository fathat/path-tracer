#pragma once

#include "types.h"
#include <SDL.h>
#include <SDL_render.h>
#include "image_buffer.h"

class streaming_image_texture_t {
public:
    streaming_image_texture_t(SDL_Renderer* renderer, int width, int height);
    ~streaming_image_texture_t();
        
    // delete all the default constructors, we don't want to copy or move this
    // (best to put it in a shared_ptr really)
    streaming_image_texture_t(streaming_image_texture_t&& m) = delete;
    streaming_image_texture_t(const streaming_image_texture_t& c) = delete;
    streaming_image_texture_t operator=(const streaming_image_texture_t& t) = delete;
    streaming_image_texture_t& operator=(streaming_image_texture_t&& m) = delete;

    [[nodiscard]] SDL_Texture* texture() const { return m_texture; }
    [[nodiscard]] image_buffer_t* image() const { return m_buffer.get(); }
    [[nodiscard]] SDL_Renderer* renderer() const { return m_renderer; }
    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }

    void update_texture_sync();

    void present();

    void resize(int width, int height);

protected:
    int m_width;
    int m_height;
    unique_ptr<image_buffer_t> m_buffer;

    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;
    uint32_t m_format;
};
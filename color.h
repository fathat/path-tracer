#pragma once
#include <cstdint>
#include <memory>

struct Color
{
    double r, g, b, a;
};

class ImageBuffer
{
public:
    ImageBuffer() = delete;

    ImageBuffer(const size_t _w, const size_t _h)
    : w(_w), h(_h)
    {
        buffer = std::make_unique<uint8_t[]>(w*h*4);
    }

    inline void write(const size_t x, const size_t y, const Color& color)
    {
        const size_t baseIndex = x * 4 + y * w * 4;

        buffer[baseIndex] = static_cast<uint8_t>(color.a * 255);
        buffer[baseIndex + 1] = static_cast<uint8_t>(color.b * 255);
        buffer[baseIndex + 2] = static_cast<uint8_t>(color.g * 255);
        buffer[baseIndex + 3] = static_cast<uint8_t>(color.r * 255);
    }

    [[nodiscard]] uint8_t* data() const { return buffer.get(); }

    [[nodiscard]] size_t pitch() const { return w * 4; }

    [[nodiscard]] size_t width() const { return w; }
    [[nodiscard]] size_t height() const { return h; }

protected:
    size_t w;
    size_t h;

    std::unique_ptr<uint8_t[]> buffer;
};



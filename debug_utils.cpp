#include <iostream>
#include "types.h"
#include "debug_utils.h"

void render_gradient_pattern(image_buffer_t& buffer) {
#ifdef THREADS
    std::lock_guard guard(*buffer.mutex());
#endif
    for (int y = 0; y < buffer.height(); y++)
    {
        for (int x = 0; x < buffer.width(); x++)
        {
            const double r = static_cast<double>(x) / static_cast<double>(buffer.width() - 1);
            const double g = static_cast<double>(y) / static_cast<double>(buffer.height() - 1);
            constexpr double b = 0.25;
            constexpr double a = 1.0;

            buffer.write(x, y, {r,g,b,a}, 1);
        }
    }
}

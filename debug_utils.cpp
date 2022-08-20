#include <iostream>
#include "debug_utils.h"

void render_test_pattern(image_buffer& buffer) {
    for (int y = 0; y < buffer.height(); y++)
    {
        std::cout << "\rScanlines remaining: " << (buffer.height()-1) - y << std::endl;
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

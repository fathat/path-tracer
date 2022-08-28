#pragma once
#include <cstdint>
#include <memory>
#include "types.h"

/**
 * \brief Represents a color. At some point this will be updated to
 * be HDR aware, although at the moment it's best to assume the range
 * is 0.0-1.0. Note that on most math operations, alpha is treated
 * differently from RGB (for instance, multiplying by a scalar will
 * multiply RGB, but leave alpha out). 
 */
struct color_t
{
    double a, b, g, r;

    color_t()
        : r(0), g(0), b(0), a(1.0)
    {}

    color_t(const double red, const double green, const double blue, const double alpha=1.0)
    : r(red), g(green), b(blue), a(alpha) {}

    static color_t from_int(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha=255) {
        return {
            static_cast<double>(red) / 255.0,
            static_cast<double>(green) / 255.0,
            static_cast<double>(blue) / 255.0,
            static_cast<double>(alpha) / 255.0
        };
    }

    static color_t random(double min = 0.0, double max = 1.0) {
        return {
            random_double(min, max),
            random_double(min, max),
            random_double(min, max)
        };
    }
};

inline color_t operator*(const color_t& c, const double t) {
    return {c.r * t, c.g * t, c.b * t, c.a};
}

inline color_t operator*(const double t, const color_t& c) {
    return {c.r * t, c.g * t, c.b * t, c.a};
}

inline color_t operator*(const color_t& c1, const color_t& c2) {
    return {c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a};
}

inline color_t operator/(const color_t& c1, const color_t& c2) {
    return {c1.r / c2.r, c1.g / c2.g, c1.b / c2.b, c1.a / c2.a};
}

inline color_t operator/(const color_t& c1, const double& d) {
    return {c1.r / d, c1.g / d, c1.b / d, c1.a};
}

inline color_t operator+(const color_t& c1, const color_t& c2) {
    return {c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, (c1.a + c2.a) * 0.5};
}

inline color_t operator-(const color_t& c1, const color_t& c2) {
    return {c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, (c1.a + c2.a) * 0.5};
}

inline color_t operator+(const dvec3_t& v, const color_t& c) {
    return {
        v.x + c.r,
        v.y + c.g,
        v.z + c.b,
        c.a
    };
}

inline color_t operator+=(color_t& c, const color_t& c2) {
    c.r += c2.r;
    c.g += c2.g;
    c.b += c2.b;
    return c;
}

inline color_t operator-=(color_t& c, const color_t& c2) {
    c.r -= c2.r;
    c.g -= c2.g;
    c.b -= c2.b;
    return c;
}


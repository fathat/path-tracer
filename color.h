#pragma once
#include <cstdint>
#include <memory>
#include "types.h"

struct color
{
    double r, g, b, a;

    color()
        : r(0), g(0), b(0), a(1.0)
    {}

    color(const double red, const double green, const double blue, const double alpha=1.0)
    : r(red), g(green), b(blue), a(alpha) {}

    static color from_int(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha=255) {
        return {
            static_cast<double>(red) / 255.0,
            static_cast<double>(green) / 255.0,
            static_cast<double>(blue) / 255.0,
            static_cast<double>(alpha) / 255.0
        };
    }
};

inline color operator*(const color& c, const double t) {
    return {c.r * t, c.g * t, c.b * t, c.a};
}

inline color operator*(const double t, const color& c) {
    return {c.r * t, c.g * t, c.b * t, c.a};
}

inline color operator*(const color& c1, const color& c2) {
    return {c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a};
}

inline color operator/(const color& c1, const color& c2) {
    return {c1.r / c2.r, c1.g / c2.g, c1.b / c2.b, c1.a / c2.a};
}


inline color operator+(const color& c1, const color& c2) {
    return {c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, (c1.a + c2.a) * 0.5};
}

inline color operator-(const color& c1, const color& c2) {
    return {c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, (c1.a + c2.a) * 0.5};
}

inline color operator+(const vec3_d& v, const color& c) {
    return {
        v.x + c.r,
        v.y + c.g,
        v.z + c.b,
        c.a
    };
}

inline color operator+=(color& c, const color& c2) {
    c.r += c2.r;
    c.g += c2.g;
    c.b += c2.b;
    return c;
}

inline color operator-=(color& c, const color& c2) {
    c.r -= c2.r;
    c.g -= c2.g;
    c.b -= c2.b;
    return c;
}


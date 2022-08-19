#pragma once

#include "types.h"

class Ray {
public:

    Ray() {}

    Ray(const vec3_d& origin, const vec3_d& direction)
        : orig(origin), dir(direction) {
        
    }

    [[nodiscard]] vec3_d origin() const { return orig; }
    [[nodiscard]] vec3_d direction() const { return dir; }

    [[nodiscard]] vec3_d at(double t) const {
        return orig + t*dir;
    }

protected:
    vec3_d orig;
    vec3_d dir;
};
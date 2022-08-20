#pragma once

#include <cstdlib>
#include <memory>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

typedef glm::vec<3, double, glm::highp>	vec4_d;
typedef glm::vec<3, double, glm::highp>	vec3_d;
typedef glm::vec<2, double, glm::highp>	vec2_d;

typedef glm::vec<3, double, glm::highp>	point3;

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;

inline double degrees_to_radians(const double degrees) {
    constexpr double pi_over_180 = pi / 180.0;
    return degrees * pi_over_180;
}
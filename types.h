#pragma once

#include <cstdlib>
#include <memory>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

#ifndef EMSCRIPTEN
#define THREADS
#endif

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

using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using glm::length2;

const double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;

inline double degrees_to_radians(const double degrees) {
    constexpr double pi_over_180 = pi / 180.0;
    return degrees * pi_over_180;
}

inline vec3_d random_vec3() {
    return {random_double(), random_double(), random_double()};
}

inline vec3_d random_vec3(double min, double max) {
    return {
        random_double(min, max),
        random_double(min, max),
        random_double(min, max)};
}

inline vec3_d random_in_unit_sphere() {
    while(true) {
        auto p = random_vec3(-1, 1);
        if(length2(p) >= 1) continue;
        return p;
    }
}

inline vec3_d random_unit_vector() {
    auto p = random_vec3(-1, 1);
    return glm::normalize(p);
}

inline vec3_d random_in_unit_disk() {
    while (true) {
        auto p = vec3_d(random_double(-1,1), random_double(-1,1), 0);
        if (length2(p) >= 1) continue;
        return p;
    }
}

inline bool near_zero(const vec3_d& v) {
    const auto s = 1e-8;
    return (fabs(v.x) < s) && (fabs(v.y) < s) && (fabs(v.z) < s);
}

inline vec3_d reflect(const vec3_d& v, const vec3_d& n) {
    return v - 2*dot(v,n)*n;
}

inline vec3_d refract(const vec3_d& uv, const vec3_d& n, const double etai_over_etat) {
    const auto cos_theta = fmin(dot(-uv, n), 1.0);
    const vec3_d r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    const vec3_d r_out_parallel = -sqrt(fabs(1.0 - length2(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}
#pragma once

#include <iostream>
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

inline double random_double(const double min, const double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline int random_int(const int min, const int max) {
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max+1));
}

// glm usually works with floats by default, so we want some
// types to work on doubles instead
typedef glm::vec<3, double, glm::highp>	dvec4_t;
typedef glm::vec<3, double, glm::highp>	dvec3_t;
typedef glm::vec<2, double, glm::highp>	dvec2_t;

typedef glm::vec<3, double, glm::highp>	point3;

typedef glm::mat<4, 4, double, glm::highp> dmat4_t;


using glm::dquat;

using std::cout;
using std::endl;
using std::stringstream;
using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using std::make_unique;
using std::sqrt;
using glm::length2;

const double infinity = std::numeric_limits<double>::infinity();
constexpr double g_pi = 3.1415926535897932385;

inline double degrees_to_radians(const double degrees) {
    constexpr double pi_over_180 = g_pi / 180.0;
    return degrees * pi_over_180;
}

inline dvec3_t random_vec3() {
    return {random_double(), random_double(), random_double()};
}

inline dvec3_t random_vec3(double min, double max) {
    return {
        random_double(min, max),
        random_double(min, max),
        random_double(min, max)};
}

inline dvec3_t random_in_unit_sphere() {
    while(true) {
        auto p = random_vec3(-1, 1);
        if(length2(p) >= 1) continue;
        return p;
    }
}

inline dvec3_t random_unit_vector() {
    auto p = random_vec3(-1, 1);
    return glm::normalize(p);
}

inline dvec3_t random_in_unit_disk() {
    const auto d = normalize(dvec3_t(random_double(-1,1), random_double(-1,1), 0));
    const auto l = random_double(0.001, 1.0);
    return d*l;
    /*while (true) {
        auto p = dvec3_t(random_double(-1,1), random_double(-1,1), 0);
        if (length2(p) >= 1) continue;
        return p;
    }*/
}

inline bool near_zero(const dvec3_t& v) {
    const auto s = 1e-8;
    return (fabs(v.x) < s) && (fabs(v.y) < s) && (fabs(v.z) < s);
}

inline dvec3_t reflect(const dvec3_t& v, const dvec3_t& n) {
    return v - 2*dot(v,n)*n;
}

inline dvec3_t refract(const dvec3_t& uv, const dvec3_t& n, const double etai_over_etat) {
    const auto cos_theta = fmin(dot(-uv, n), 1.0);
    const dvec3_t r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    const dvec3_t r_out_parallel = -sqrt(fabs(1.0 - length2(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

dmat4_t create_transform_matrix(const dvec3_t& location, const dquat& rotation);

point3 transform_point(const point3& p, const dmat4_t& transform);
dvec3_t transform_vec(const dvec3_t& p, const dmat4_t& transform);
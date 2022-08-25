#include "sphere.h"
#include "ray.h"

using namespace glm;

inline double length_sq(const vec3_d& v) {
    return v.x*v.x + v.y*v.y;
}

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    const vec3_d oc = r.origin() - center;
    const auto a = length_sq(r.direction());
    const auto half_b = dot(oc, r.direction());
    const auto c = length_sq(oc) - radius*radius;

    const auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    const auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    const vec3_d outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;

    return true;
}

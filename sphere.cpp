#include "sphere.h"
#include "ray.h"

using namespace glm;

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    const vec3_d oc = r.origin() - center;
    const auto a = length2(r.direction());
    const auto half_b = dot(oc, r.direction());
    const auto c = length2(oc) - radius*radius;

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
    rec.normal = (rec.p - center) / radius;
    rec.mat = mat;

    return true;
}
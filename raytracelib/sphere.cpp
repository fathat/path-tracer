#include "sphere.h"
#include "ray.h"

using namespace glm;

dvec2_t get_sphere_uv(const point3& p) {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    auto theta = acos(-p.y);
    auto phi = atan2(-p.z, p.x) + g_pi;

    double u = phi / (2*g_pi);
    double v = theta / g_pi;
    return {u, v};
}

bool sphere_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const {
    const dvec3_t oc = r.origin() - center(r.time());
    const auto a = length2(r.direction());
    const auto half_b = dot(oc, r.direction());
    const auto c = length2(oc) - m_radius*m_radius;

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
    const dvec3_t outward_normal = (rec.p - center(r.time())) / m_radius;
    rec.set_face_normal(r, outward_normal);
    rec.uv = get_sphere_uv(outward_normal);
    rec.mat = m_mat;

    return true;
}

bool sphere_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    
    aabb_t box0(
        center(time0) - dvec3_t(m_radius, m_radius, m_radius),
        center(time0) + dvec3_t(m_radius, m_radius, m_radius));
    aabb_t box1(
        center(time1) - dvec3_t(m_radius, m_radius, m_radius),
        center(time1) + dvec3_t(m_radius, m_radius, m_radius));
    output_box = surrounding_box(box0, box1);
    return true;
}

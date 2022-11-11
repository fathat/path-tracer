#include "rect.h"

bool xy_rect_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const {
    auto t = (k-r.origin().z) / r.direction().z;
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x + t*r.direction().x;
    auto y = r.origin().y + t*r.direction().y;
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    rec.uv = dvec2_t((x-x0)/(x1-x0),(y-y0)/(y1-y0));
    rec.t = t;
    auto outward_normal = dvec3_t(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mp;
    rec.p = r.at(t);
    return true;
}
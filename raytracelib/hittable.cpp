#include "hittable.h"
#include "ray.h"

void hit_record_t::set_face_normal(const ray_t& r, const dvec3_t& outward_normal) {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal :-outward_normal;
}

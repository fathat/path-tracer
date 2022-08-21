#include "hittable.h"
#include "ray.h"

void hit_record::set_face_normal(const ray& r, const vec3_d& outward_normal) {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal :-outward_normal;
}

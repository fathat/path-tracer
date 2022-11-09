#include "hittable_list.h"

void hittable_list_t::clear() {
    objects.clear();
}

void hittable_list_t::add(const shared_ptr<hittable_t>& object) {
    objects.push_back(object);
}

bool hittable_list_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const {
    hit_record_t temp_rec{};
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

bool hittable_list_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    if (objects.empty()) return false;

    aabb_t temp_box;
    bool first_box = true;

    for (const auto& object : objects) {
        if (!object->bounding_box(time0, time1, temp_box)) return false;
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }

    return true;
}

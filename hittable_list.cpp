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

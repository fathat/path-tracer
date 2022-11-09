#include "bvh_node.h"

#include <algorithm>

double get_axis(point3 p, int _axis) {
    if(_axis == 0) {
        return p.x;
    } else if(_axis == 1) {
        return p.y;
    } else if(_axis == 2) {
        return p.z;
    }
    return 0.0;
}

bool box_compare(const shared_ptr<hittable_t>& a, const shared_ptr<hittable_t>& b, int axis) {
    aabb_t box_a;
    aabb_t box_b;

    if (!a->bounding_box(0,0, box_a) || !b->bounding_box(0,0, box_b))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return get_axis(box_a.min(), axis) < get_axis(box_b.min(), axis);
}


bool box_x_compare (const shared_ptr<hittable_t> a, const shared_ptr<hittable_t> b) {
    return box_compare(a, b, 0);
}

bool box_y_compare (const shared_ptr<hittable_t> a, const shared_ptr<hittable_t> b) {
    return box_compare(a, b, 1);
}

bool box_z_compare (const shared_ptr<hittable_t> a, const shared_ptr<hittable_t> b) {
    return box_compare(a, b, 2);
}


bool bvh_node_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    output_box = box;
    return true;
}

bvh_node_t::bvh_node_t(const std::vector<shared_ptr<hittable_t>>& src_objects, size_t start, size_t end, double time0,
    double time1) {
    auto objects = src_objects; // Create a modifiable array of the source scene objects

    int axis = random_int(0,2);
    auto comparator = (axis == 0) ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

    size_t object_span = end - start;

    if (object_span == 1) {
        left = right = objects[start];
    } else if (object_span == 2) {
        if (comparator(objects[start], objects[start+1])) {
            left = objects[start];
            right = objects[start+1];
        } else {
            left = objects[start+1];
            right = objects[start];
        }
    } else {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + object_span/2;
        left = make_shared<bvh_node_t>(objects, start, mid, time0, time1);
        right = make_shared<bvh_node_t>(objects, mid, end, time0, time1);
    }

    aabb_t box_left, box_right;

    if (  !left->bounding_box (time0, time1, box_left)
       || !right->bounding_box(time0, time1, box_right)
    )
        std::cerr << "No bounding box in bvh_node constructor.\n";

    box = surrounding_box(box_left, box_right);
    
}

bool bvh_node_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const {
    if (!box.hit(r, t_min, t_max))
        return false;

    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

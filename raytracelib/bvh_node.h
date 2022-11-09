#pragma once

#include "hittable.h"
#include "hittable_list.h"


class bvh_node_t : public hittable_t {
    public:
        bvh_node_t();

        bvh_node_t(const hittable_list_t& list, double time0, double time1)
            : bvh_node_t(list.objects, 0, list.objects.size(), time0, time1)
        {}

        bvh_node_t(
            const std::vector<shared_ptr<hittable_t>>& src_objects,
            size_t start, size_t end, double time0, double time1);

        virtual bool hit(
            const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const override;

    public:
        shared_ptr<hittable_t> left;
        shared_ptr<hittable_t> right;
        aabb_t box;
};


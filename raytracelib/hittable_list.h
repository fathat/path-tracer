#pragma once
#include <memory>
#include <vector>

#include "hittable.h"

using std::shared_ptr;
using std::make_shared;
using std::vector;

class hittable_list_t : public hittable_t {
    public:
        hittable_list_t() = default;
        explicit hittable_list_t(const shared_ptr<hittable_t>& object) { add(object); }

        void clear();
        void add(const shared_ptr<hittable_t>& object);

        virtual bool hit(
            const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

        virtual bool bounding_box(
            double time0, double time1, aabb_t& output_box) const override;

    public:
        vector<shared_ptr<hittable_t>> objects;
};

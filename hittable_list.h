#pragma once
#include <memory>
#include <vector>

#include "hittable.h"

using std::shared_ptr;
using std::make_shared;
using std::vector;

class hittable_list_t : public hittable {
    public:
        hittable_list_t() = default;
        explicit hittable_list_t(const shared_ptr<hittable>& object) { add(object); }

        void clear();
        void add(const shared_ptr<hittable>& object);

        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

    public:
        vector<shared_ptr<hittable>> objects;
};

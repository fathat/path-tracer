#pragma once

#include "types.h"
#include "hittable.h"

class xy_rect_t : public hittable_t {
    public:
        xy_rect_t() {}

        xy_rect_t(double _x0, double _x1, double _y0, double _y1, double _k, 
            shared_ptr<material_t> mat)
            : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};

        virtual bool hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the Z
            // dimension a small amount.
            output_box = aabb_t(point3(x0,y0, k-0.0001), point3(x1, y1, k+0.0001));
            return true;
        }

    public:
        shared_ptr<material_t> mp;
        double x0, x1, y0, y1, k;
};

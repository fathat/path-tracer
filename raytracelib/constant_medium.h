#pragma once
#include "hittable.h"
#include "material.h"
#include "texture.h"

class constant_medium_t : public hittable_t {
    public:
        constant_medium_t(shared_ptr<hittable_t> b, double d, shared_ptr<texture_t> a)
            : boundary(b),
              neg_inv_density(-1/d),
              phase_function(make_shared<isotropic_material_t>(a))
            {}

        constant_medium_t(shared_ptr<hittable_t> b, double d, color_t c)
            : boundary(b),
              neg_inv_density(-1/d),
              phase_function(make_shared<isotropic_material_t>(c))
            {}

        virtual bool hit(
            const ray_t& r, double t_min, double t_max, hit_record_t& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb_t& output_box) const override {
            return boundary->bounding_box(time0, time1, output_box);
        }

    public:
        shared_ptr<hittable_t> boundary;
        shared_ptr<material_t> phase_function;
        double neg_inv_density;
};

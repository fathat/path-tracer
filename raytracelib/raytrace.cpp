#include "raytrace.h"

color_t ray_color(const ray_t& r, const hittable_t& world, const int stack_depth, const int max_bounces) {
    hit_record_t rec{};
    
    if(stack_depth > max_bounces) {
        return {0, 0, 0, 1};
    }

    if (world.hit(r, 0.001, infinity, rec)) {
        ray_t scattered;
        color_t attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, stack_depth+1, max_bounces);
        return {0,0,0};
    }
    const dvec3_t unit_direction = normalize(r.direction());
    auto t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*color_t(1.0, 1.0, 1.0) + t * color_t(0.5, 0.7, 1.0);
}

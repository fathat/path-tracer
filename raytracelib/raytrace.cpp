#include "raytrace.h"

color_t ray_color(const ray_t& r, const scene_t& scene, const hittable_t& world, int depth) {
    hit_record_t rec{};
    
    if(depth <= 0) {
        return {0, 0, 0, 1};
    }

    if (!world.hit(r, 0.001, infinity, rec)) {
        return scene.background;
    }

    ray_t scattered;
    color_t attenuation;
    color_t emitted = rec.mat->emitted(rec.uv.x, rec.uv.y, rec.p);

    if (!rec.mat->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, scene, world, depth-1);
}

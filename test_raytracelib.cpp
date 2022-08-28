#include <gtest/gtest.h>

#include "raytracelib/ray.h"
#include "raytracelib/sphere.h"
#include "raytracelib/material.h"

TEST(SphereTest, BasicCollision) {
    ray_t r {};
    const auto mat = make_shared<lambertian_material_t>(color_t{1.0, 0.0, 0.0, 1.0});
    sphere_t sphere({0, 0, 10}, 5.0, mat);

    hit_record_t hit_record;
    bool was_hit = sphere.hit(r, 0, infinity, hit_record);
    EXPECT_TRUE(was_hit);

}
#include <gtest/gtest.h>

#include "raytracelib/ray.h"
#include "raytracelib/sphere.h"
#include "raytracelib/material.h"
#include "raytracelib/rect.h"

using namespace glm;

bool double_eq(double x, double y) {
    if(fabs(x - y) < 0.00001) {
        return true;
    }
    return false;
}


TEST(SphereTest, BasicCollision) {
    ray_t r {};
    const auto mat = make_shared<lambertian_material_t>(color_t{1.0, 0.0, 0.0, 1.0});
    sphere_t sphere({0, 0, 10}, 5.0, mat);

    hit_record_t hit_record;
    bool was_hit = sphere.hit(r, 0, infinity, hit_record);
    EXPECT_TRUE(was_hit);

}

TEST(RayTest, RayRotation) {
    ray_t r { {0.0,0.0,0.0}, {0.0, 0.0, 1.0} };

    auto rotation = glm::angleAxis<float, glm::qualifier::defaultp>((float)degrees_to_radians(90.0), {0.0f, 1.0f, 0.0f});
    auto new_ray = r.transformed({}, rotation);
    ASSERT_DOUBLE_EQ(new_ray.direction().x, 1.0f);
}

TEST(RayTest, RayLookAt) {
    ray_t r { {0.0,0.0,0.0}, {0.0, 0.0, 1.0} };

    auto rotation = glm::quatLookAt<float, glm::qualifier::defaultp>({-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    auto new_ray = r.transformed({}, rotation);
    ASSERT_DOUBLE_EQ(new_ray.direction().x, 1.0f);
}

TEST(RayTest, RayTranslate) {
    ray_t r { {0.0,0.0,0.0}, {0.0, 0.0, 1.0} };

    auto transform = glm::translate(glm::identity<glm::mat4x4>(), {5.0f, 0.0f, 0.0f});
    auto new_ray = r.transformed(transform);
    ASSERT_DOUBLE_EQ(new_ray.origin().x, 5.0f);
}

TEST(RayTest, RayRotate) {
    ray_t r { {0.0,0.0,0.0}, {0.0, 0.0, 1.0} };
        
    //glm::mat4x4 translate = glm::translate(glm::identity<glm::mat4x4>(), {5.0f, 0.0f, 0.0f});
    dmat4_t rotate = glm::rotate(identity<dmat4_t>(), glm::radians(90.0), {0.0, 1.0, 0.0});
    auto new_ray = r.transformed(rotate);
    ASSERT_DOUBLE_EQ(new_ray.direction().x, 1.0f);
    ASSERT_TRUE(double_eq(new_ray.direction().z , 0.0f));
}

TEST(RayTest, RayTranslateAndRotate) {
    ray_t r { {0.0,0.0,0.0}, {0.0, 0.0, 1.0} };
        
    dmat4_t translation = translate(identity<dmat4>(), {5.0, 0.0, 0.0});
    dmat4_t rotation = rotate(identity<dmat4_t>(), glm::radians(90.0), {0.0, 1.0, 0.0});
    auto new_ray = r.transformed( translation * rotation);
    ASSERT_DOUBLE_EQ(new_ray.direction().x, 1.0);
    ASSERT_TRUE(double_eq(new_ray.direction().z , 0.0));
    ASSERT_TRUE(double_eq(new_ray.origin().x , 5.0));
}

TEST(RectTest, RayAtOrigin) {
    ray_t r { {0.0,0.0,1.0}, {0.0, 0.0, -1.0} };
    rect_t rect {1, 1, {0.0, 0.0, 0.0}, {}, nullptr};
    hit_record_t record;

    bool was_hit = rect.hit(r, 0.0001, infinity,  record);
    ASSERT_TRUE(was_hit);
}
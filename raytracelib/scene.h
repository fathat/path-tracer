#pragma once
#include <memory>
#include "bvh_node.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

struct scene_t {
    scene_t(const camera_t& camera): cam(camera) {}
    scene_t(const camera_t& camera, const hittable_list_t& ents): entities(ents), cam(camera) {}
    hittable_list_t entities;
    shared_ptr<bvh_node_t> root;
    camera_t cam;
    color_t background = {0, 0, 0};
};


scene_t random_scene(int image_width, int image_height);

scene_t three_spheres_scene(int image_width, int image_height);

scene_t earth_scene(int image_width, int image_height);

scene_t two_perlin_spheres_scene(int image_width, int image_height);

scene_t simple_light(int image_width, int image_height);

scene_t simple_box(int image_width, int image_height);

scene_t box_test(int image_width, int image_height);

scene_t cornell_box(int image_width, int image_height);
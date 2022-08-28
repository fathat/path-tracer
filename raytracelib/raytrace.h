#pragma once

#include "camera.h"
#include "debug_utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "types.h"
#include "image_buffer.h"
#include "ray.h"
#include "sphere.h"
#include "material.h"
#include "scene.h"

/**
 * \brief The main ray tracing function
 * \param r The ray to trace 
 * \param world The things to collide with
 * \param stack_depth How many bounces have we gone through
 * \param max_bounces The maximum number of bounces allowed
 * \return A color
 */
color_t ray_color(const ray_t& r, const hittable_t& world, const int stack_depth, const int max_bounces);

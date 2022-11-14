#include "box.h"

box_t::box_t(dvec3_t center, dquat rotation, double width, double height, double depth, const shared_ptr<material_t>& mat):
    m_center(center), m_rotation(rotation), m_width(width), m_height(height), m_depth(depth)
{
    auto w2 = width/2;
    auto h2 = height/2;
    auto d2 = depth/2;
    auto r90y = glm::angleAxis<double, glm::highp>(glm::radians(90.0), {0.0, 1.0, 0.0});
    auto r90x = glm::angleAxis<double, glm::highp>(glm::radians(90.0), {1.0, 0.0, 0.0});

    // all planes in local space
    m_front = make_shared<rect_t>(width, height, dvec3_t{0, 0, d2}, glm::quat(), mat);
    m_back = make_shared<rect_t>(width, height, dvec3_t{0, 0, -d2}, glm::quat(), mat);
    m_left = make_shared<rect_t>(depth, height, dvec3_t{-w2, 0, 0}, r90y, mat);
    m_right = make_shared<rect_t>(depth, height,dvec3_t{w2, 0, 0}, r90y, mat);
    m_top = make_shared<rect_t>(width, depth, dvec3_t{0, h2, 0}, r90x, mat);
    m_bottom = make_shared<rect_t>(width, depth, dvec3_t{0, -h2, 0}, r90x, mat);

    m_cached_transform = create_transform_matrix(m_center, m_rotation);
    m_cached_inverse_transform = glm::inverse(m_cached_transform);
}

bool box_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& out) const {

    //translate to local space
    ray_t local_ray = r.transformed(m_cached_inverse_transform);

    std::vector<hit_record_t> hits;

    for(auto& surface : {m_front, m_back, m_left, m_right, m_top, m_bottom}) {
        hit_record_t rec;
        if(surface->hit(local_ray, t_min, t_max, rec)) {
            hits.push_back(rec);
        }
    }

    // find the closest hit
    if(!hits.empty()) {
        hit_record_t* closest_hit = &hits[0];

        for(size_t i=1; i<hits.size(); i++) {
            if(hits[i].t < closest_hit->t) {
                closest_hit = &hits[i];
            }
        }

        closest_hit->p  = transform_point(closest_hit->p, m_cached_transform);
        closest_hit->normal = transform_vec(closest_hit->normal, m_cached_transform);

        out = *closest_hit;
    }
    
    return !hits.empty();
}

bool box_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    aabb_t box;

    for(auto& surface : {m_front, m_back, m_left, m_right, m_top, m_bottom}) {
        aabb_t surface_bb;
        if(surface->bounding_box(time0, time1, surface_bb)) {
            box.expand(surface_bb);
        }
    }
    output_box = box;
    return true;
}

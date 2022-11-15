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

    m_sides.add(m_front);
    m_sides.add(m_back);
    m_sides.add(m_left);
    m_sides.add(m_right);
    m_sides.add(m_top);
    m_sides.add(m_bottom);

    m_cached_transform = create_transform_matrix(m_center, m_rotation);
    m_cached_inverse_transform = glm::inverse(m_cached_transform);

    //calculate vertices
    std::vector<dvec3_t> vertices = {
        {-w2, -h2, -d2},
        { w2, -h2, -d2},
        { w2, -h2, d2},
        {-w2, -h2, d2},
        {-w2, h2, -d2},
        { w2, h2, -d2},
        { w2, h2, d2},
        {-w2, h2, d2},
    };

    for(const auto& v : vertices) {
        m_vertices.push_back(transform_point(v, m_cached_transform));
    }

    m_cached_bb = aabb_t(m_vertices);
}

bool box_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& out) const {

    //translate to local space
    ray_t local_ray = r.transformed(m_cached_inverse_transform);

    bool hit = m_sides.hit(local_ray, t_min, t_max, out);
    if(hit) {
        out.p = transform_point(out.p, m_cached_transform);
        out.normal = transform_vec(out.normal, m_cached_transform);
    }
    
    return hit;
}

bool box_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    output_box = m_cached_bb;
    return true;
}

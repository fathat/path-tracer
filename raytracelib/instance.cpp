#include "instance.h"

instance_t::instance_t(shared_ptr<hittable_t> source, const dmat4_t& transform):
    m_source(source),
    m_transform(transform)
{
    m_inverse = glm::inverse(m_transform);
    aabb_t bb;
    m_source->bounding_box(0, 0, bb);
    std::vector<point3> point_cloud;
    transform_points(bb.vertices(), m_transform, point_cloud);
    m_aabb = aabb_t(point_cloud);
}

bool instance_t::hit(const ray_t& r, double t_min, double t_max, hit_record_t& rec) const {

    auto local_ray = r.transformed(m_inverse);
    hit_record_t local_rec;
    if(m_source->hit(local_ray, t_min, t_max, local_rec)) {
        rec = local_rec;
        rec.p = transform_point(local_rec.p, m_transform);
        rec.normal = transform_vec(local_rec.normal, m_transform);
        return true;
    }
    return false;
}

bool instance_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    output_box = m_aabb;
    return true;
}

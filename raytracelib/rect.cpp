#include "rect.h"
#include <glm/gtc/quaternion.hpp>

double extract_axis(const dvec3_t& v, int axis) {
    if(axis == 0) {
        return v.x;
    } else if(axis == 1) {
        return v.y;
    } else {
        return v.z;
    }
}

double min_val(const std::vector<dvec3_t>& points, int axis) {
    double m = std::numeric_limits<double>::max();
    for(const auto& p : points) {
        auto v = extract_axis(p, axis);
        if(v < m) {
            m = v;
        }
    }
    return m;
}

double max_val(const std::vector<dvec3_t>& points, int axis) {
    double m = -std::numeric_limits<double>::max();
    for(const auto& p : points) {
        auto v = extract_axis(p, axis);
        if(v > m) {
            m = v;
        }
    }
    return m;
}

rect_t::rect_t(double w, double h, dvec3_t center, glm::quat rotation, const shared_ptr<material_t>& mat): m_width(w), m_height(h), m_material(mat), m_center(center), m_rotation(rotation) {
    calc_transform();
    calc_bounding_box();
}

void rect_t::calc_transform() {
    m_cached_transform = create_transform_matrix(m_center, m_rotation);
    m_cached_inverse_transform = glm::inverse(m_cached_transform);
}

void rect_t::calc_bounding_box() {
    double w2 = m_width/2;
    double h2 = m_height/2;
    dvec3_t a = point3(-w2, -h2, 0.0);
    dvec3_t b = point3(w2, -h2, 0.0);
    dvec3_t c = point3(w2, h2, 0.0);
    dvec3_t d = point3(-w2, h2, 0.0);

    dvec3_t points[] = {a,b,c,d};
    std::vector<point3> transformed_points;
        
    for(auto p : points) {
        glm::vec4 v = {p.x, p.y, p.z, 0};
        auto rotated = m_rotation * v;
        auto transformed = dvec3_t(rotated.x, rotated.y, rotated.z) + m_center;
        transformed_points.push_back(transformed);
    }


    double xmin = min_val(transformed_points, 0) - 0.001;
    double ymin = min_val(transformed_points, 1) - 0.001;
    double zmin = min_val(transformed_points, 2) - 0.001;

    double xmax = max_val(transformed_points, 0) + 0.001;
    double ymax = max_val(transformed_points, 1) + 0.001;
    double zmax = max_val(transformed_points, 2) + 0.001;

    m_cached_bb = aabb_t(point3(xmin, ymin, zmin), point3(xmax, ymax, zmax));
}

bool rect_t::hit(const ray_t& ray_original, double t_min, double t_max, hit_record_t& rec) const {
    // TODO add time
    auto k = 0;
    double w2 = m_width/2;
    double h2 = m_height/2;

    double x0 = -w2;
    double x1 = w2;
    double y0 = -h2;
    double y1 = h2;

    auto tform = transform();
    auto inv_tform = inverse_transform();

    auto local_ray = ray_original.transform(inv_tform);

    auto t = (k-local_ray.origin().z) / local_ray.direction().z;
    if (t < t_min || t > t_max)
        return false;
    auto x = local_ray.origin().x + t*local_ray.direction().x;
    auto y = local_ray.origin().y + t*local_ray.direction().y;
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    rec.uv = dvec2_t((x-x0)/(x1-x0),(y-y0)/(y1-y0));

    const point3 local_point = local_ray.at(t);
    const point3 world_point = transform_point(local_point, tform);

    rec.t = t;
    auto outward_normal = tform * glm::vec4(0, 0, 1, 0);
    rec.set_face_normal(ray_original, outward_normal);
    rec.mat = m_material;
    rec.p = world_point;
    return true;
}

bool rect_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    //TODO: incorporate time
    output_box = m_cached_bb;
    return true;
}

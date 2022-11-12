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

dmat4_t rect_t::transform() const {
    const dmat4_t t = glm::translate(glm::identity<dmat4_t>(), location);
    const dmat4_t r = glm::toMat4(rotation);
    return t * r;
}

bool rect_t::hit(const ray_t& ray_original, double t_min, double t_max, hit_record_t& rec) const {
    auto k = 0;
    double w2 = width/2;
    double h2 = height/2;

    double x0 = -w2;
    double x1 = w2;
    double y0 = -h2;
    double y1 = h2;

    auto invTransform = glm::inverse(transform());

    auto local_ray = ray_original.transform(invTransform);

    auto t = (k-local_ray.origin().z) / local_ray.direction().z;
    if (t < t_min || t > t_max)
        return false;
    auto x = local_ray.origin().x + t*local_ray.direction().x;
    auto y = local_ray.origin().y + t*local_ray.direction().y;
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    rec.uv = dvec2_t((x-x0)/(x1-x0),(y-y0)/(y1-y0));

    dvec3_t local_point = local_ray.at(t);
    dvec4_t world_point = transform() * glm::vec<4, double, glm::qualifier::highp>{local_point.x, local_point.y, local_point.z, 1.0};

    rec.t = t;
    auto outward_normal = transform()* glm::vec4(0, 0, 1, 0);
    rec.set_face_normal(ray_original, outward_normal);
    rec.mat = mp;
    rec.p = world_point;
    return true;
}

bool rect_t::bounding_box(double time0, double time1, aabb_t& output_box) const {
    double w2 = width/2;
    double h2 = height/2;
    dvec3_t a = point3(-w2, -h2, 0.0);
    dvec3_t b = point3(w2, -h2, 0.0);
    dvec3_t c = point3(w2, h2, 0.0);
    dvec3_t d = point3(-w2, h2, 0.0);

    dvec3_t points[] = {a,b,c,d};
    std::vector<point3> transformed_points;
        
    for(auto p : points) {
        glm::vec4 v = {p.x, p.y, p.z, 0};
        auto rotated = rotation * v;
        auto transformed = dvec3_t(rotated.x, rotated.y, rotated.z) + location;
        transformed_points.push_back(transformed);
    }


    double xmin = min_val(transformed_points, 0) - 0.001;
    double ymin = min_val(transformed_points, 1) - 0.001;
    double zmin = min_val(transformed_points, 2) - 0.001;

    double xmax = max_val(transformed_points, 0) + 0.001;
    double ymax = max_val(transformed_points, 1) + 0.001;
    double zmax = max_val(transformed_points, 2) + 0.001;

    output_box = aabb_t(point3(xmin, ymin, zmin), point3(xmax, ymax, zmax));

    return true;
}

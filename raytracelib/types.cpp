#include "types.h"

dmat4_t create_transform_matrix(const dvec3_t& location, const dquat& rotation) {
    const dmat4_t translation_mat = glm::translate(glm::identity<dmat4_t>(), location);
    const dmat4_t rotation_mat = glm::toMat4(rotation);
    return translation_mat * rotation_mat;
}

point3 transform_point(const point3& p, const dmat4_t& transform) {
    const auto result = transform * glm::vec<4, double, glm::qualifier::highp>{p.x, p.y, p.z, 1.0};
    return dvec3_t{result.x, result.y, result.z};
}

void transform_points(const std::vector<point3>& points, const dmat4_t& transform, std::vector<point3>& storage) {
    for(const auto& p : points) {
        storage.push_back(transform_point(p, transform));
    }
}

dvec3_t transform_vec(const dvec3_t& p, const dmat4_t& transform) {
    const auto result = transform * glm::vec<4, double, glm::qualifier::highp>{p.x, p.y, p.z, 0.0};
    return dvec3_t{result.x, result.y, result.z};
}

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

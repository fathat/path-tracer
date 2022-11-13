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

dvec3_t transform_vec(const dvec3_t& p, const dmat4_t& transform) {
    const auto result = transform * glm::vec<4, double, glm::qualifier::highp>{p.x, p.y, p.z, 0.0};
    return dvec3_t{result.x, result.y, result.z};
}
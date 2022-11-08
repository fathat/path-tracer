#include "material.h"
#include "hittable.h"
#include "ray.h"
#include "color.h"

bool lambertian_material_t::scatter(const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered) const {
    auto scatter_direction = rec.normal + random_unit_vector();

    if(near_zero(scatter_direction)) {
        scatter_direction = rec.normal;
    }

    scattered = ray_t(rec.p, scatter_direction, r_in.time());
    attenuation = m_albedo;
    return true;
}

bool metal_material_t::scatter(const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered) const {
    const dvec3_t reflected = reflect(normalize(r_in.direction()), rec.normal);
    scattered = ray_t(rec.p, reflected + m_fuzz*random_in_unit_sphere(), r_in.time());
    attenuation = m_albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
}

bool dielectric_material_t::scatter(
    const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
) const {
    attenuation = color_t(1.0, 1.0, 1.0);
    const double refraction_ratio = rec.front_face ? (1.0/m_index_of_refraction) : m_index_of_refraction;

    const dvec3_t unit_direction = normalize(r_in.direction());
    const double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    const double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

    const bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    dvec3_t direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = ray_t(rec.p, direction, r_in.time());
    return true;
}

double dielectric_material_t::reflectance(double cosine, double ref_idx) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

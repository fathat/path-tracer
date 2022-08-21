#include "material.h"
#include "hittable.h"
#include "ray.h"
#include "color.h"

bool lambertian::scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
    auto scatter_direction = rec.normal + random_unit_vector();

    if(near_zero(scatter_direction)) {
        scatter_direction = rec.normal;
    }

    scattered = ray(rec.p, scatter_direction);
    attenuation = m_albedo;
    return true;
}

bool metal::scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
    const vec3_d reflected = reflect(normalize(r_in.direction()), rec.normal);
    scattered = ray(rec.p, reflected + m_fuzz*random_in_unit_sphere());
    attenuation = m_albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
}

bool dielectric::scatter(
    const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
) const {
    attenuation = color(1.0, 1.0, 1.0);
    const double refraction_ratio = rec.front_face ? (1.0/m_index_of_refraction) : m_index_of_refraction;

    const vec3_d unit_direction = normalize(r_in.direction());
    const double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    const double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

    const bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    vec3_d direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = ray(rec.p, direction);
    return true;
}

double dielectric::reflectance(double cosine, double ref_idx) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

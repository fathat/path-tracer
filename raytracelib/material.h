#pragma once
#include "types.h"
#include "color.h"
#include "texture.h"

class ray_t;
struct hit_record_t;

class material_t {
public:
    virtual ~material_t() = default;
    virtual bool scatter(
            const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
        ) const = 0;
};

class lambertian_material_t : public material_t {
public:
    explicit lambertian_material_t(const color_t& a) : m_albedo(make_shared<solid_color_t>(a)) {}
    lambertian_material_t(const shared_ptr<texture_t>& a) : m_albedo(a) {}

    bool scatter(
        const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
    ) const override;

    [[nodiscard]] shared_ptr<texture_t> albedo() const { return m_albedo; }
protected:
    shared_ptr<texture_t> m_albedo;
};

class metal_material_t : public material_t {
public:
    explicit metal_material_t(const color_t& albedo, const double fuzz) : m_albedo(albedo), m_fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(
        const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
    ) const override;

    [[nodiscard]] color_t albedo() const { return m_albedo; }
protected:
    color_t m_albedo;
    double m_fuzz;
};

class dielectric_material_t : public material_t {
public:
    dielectric_material_t(double index_of_refraction) : m_index_of_refraction(index_of_refraction) {}

    bool scatter(
        const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
    ) const override;

protected:
    double m_index_of_refraction;

private:
    static double reflectance(double cosine, double ref_idx);
};
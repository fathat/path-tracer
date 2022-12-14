#pragma once
#include "types.h"
#include "color.h"
#include "ray.h"
#include "texture.h"
#include "hittable.h"

class material_t {
public:
    virtual ~material_t() = default;

    [[nodiscard]] virtual color_t emitted(double u, double v, const point3& p) const {
        return color_t(0,0,0);
    }

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

class diffuse_light : public material_t  {
    public:
        diffuse_light(shared_ptr<texture_t> a) : emit(a) {}
        diffuse_light(color_t c) : emit(make_shared<solid_color_t>(c)) {}

        bool scatter(
            const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
        ) const override {
            return false;
        }

        [[nodiscard]] color_t emitted(double u, double v, const point3& p) const override {
            return emit->value(u, v, p);
        }

    public:
        shared_ptr<texture_t> emit;
};

class isotropic_material_t : public material_t {
    public:
        isotropic_material_t(color_t c) : albedo(make_shared<solid_color_t>(c)) {}
        isotropic_material_t(shared_ptr<texture_t> a) : albedo(a) {}

        virtual bool scatter(
            const ray_t& r_in, const hit_record_t& rec, color_t& attenuation, ray_t& scattered
        ) const override {
            scattered = ray_t(rec.p, random_in_unit_sphere(), r_in.time());
            attenuation = albedo->value(rec.uv.x, rec.uv.y, rec.p);
            return true;
        }

    public:
        shared_ptr<texture_t> albedo;
};
#pragma once
#include "types.h"
#include "color.h"

class ray;
struct hit_record;

class material {
public:
    virtual ~material() = default;
    virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const = 0;
};

class lambertian : public material {
public:
    explicit lambertian(const color& a) : m_albedo(a) {}

    bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override;

    [[nodiscard]] color albedo() const { return m_albedo; }
protected:
    color m_albedo;
};

class metal : public material {
public:
    explicit metal(const color& albedo, const double fuzz) : m_albedo(albedo), m_fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override;

    [[nodiscard]] color albedo() const { return m_albedo; }
protected:
    color m_albedo;
    double m_fuzz;
};

class dielectric : public material {
public:
    dielectric(double index_of_refraction) : m_index_of_refraction(index_of_refraction) {}

    bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override;

protected:
    double m_index_of_refraction;

private:
    static double reflectance(double cosine, double ref_idx);
};
#pragma once

#include "types.h"
#include "color.h"
#include "perlin.h"

class texture_t {
    public:
        virtual color_t value(double u, double v, const point3& p) const = 0;
};

class solid_color_t : public texture_t {
    public:
        solid_color_t() {}
        solid_color_t(color_t c) : color_value(c) {}

        solid_color_t(double red, double green, double blue)
          : solid_color_t(color_t(red,green,blue)) {}

        virtual color_t value(double u, double v, const dvec3_t& p) const override {
            return color_value;
        }

    private:
        color_t color_value;
};

class checker_texture_t : public texture_t {
    public:
        checker_texture_t() = default;

        checker_texture_t(shared_ptr<texture_t> _even, shared_ptr<texture_t> _odd)
            : even(_even), odd(_odd) {}

        checker_texture_t(color_t c1, color_t c2)
            : even(make_shared<solid_color_t>(c1)) , odd(make_shared<solid_color_t>(c2)) {}

        virtual color_t value(double u, double v, const point3& p) const override {
            auto sines = sin(10*p.x)*sin(10*p.y)*sin(10*p.z);
            if (sines < 0)
                return odd->value(u, v, p);
            else
                return even->value(u, v, p);
        }

    public:
        shared_ptr<texture_t> odd;
        shared_ptr<texture_t> even;
};

class image_texture_t : public texture_t {
    public:
        const static int bytes_per_pixel = 3;

        image_texture_t()
          : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

        image_texture_t(const char* filename);

        ~image_texture_t() {
            delete data;
        }

        virtual color_t value(double u, double v, const dvec3_t& p) const override;

    private:
        unsigned char *data;
        int width, height;
        int bytes_per_scanline;
};

class noise_texture_t : public texture_t {
    public:
        noise_texture_t() = default;
        noise_texture_t(double sc) : scale(sc) {}

        [[nodiscard]] virtual color_t value(double u, double v, const point3& p) const override {
            return color_t(1,1,1) * noise.noise(scale*p);
        }

    public:
        perlin_t noise;
        double scale=1.0;
};
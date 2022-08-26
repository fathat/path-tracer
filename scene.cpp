#include "scene.h"

scene_t random_scene(int image_width, int image_height) {
    
    constexpr point3 look_from(13,2,3);
    constexpr point3 look_at(0,0,0);
    constexpr vec3_d vup(0,1,0);
    constexpr auto dist_to_focus = 10.0;
    constexpr auto aperture = 0.1;

    camera cam(
        image_width, 
        image_height, 
        60.0, 
        look_from, 
        look_at, 
        vup,
        aperture,
        dist_to_focus);

    scene_t scene {cam};

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    scene.entities.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            const auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    scene.entities.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    scene.entities.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    scene.entities.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    scene.entities.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    scene.entities.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    scene.entities.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return scene;
}

scene_t test_scene(int image_width, int image_height) {
    
    constexpr point3 look_from(3,3,2);
    constexpr point3 look_at(0,0,-1);
    constexpr vec3_d vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 1.0;

    camera cam {image_width, image_height, 60.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};


    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric>(1.5);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

    scene.entities.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    scene.entities.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    scene.entities.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    scene.entities.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0), -0.45, material_left));
    scene.entities.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
    return scene;
}

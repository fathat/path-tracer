#include "scene.h"

#include "box.h"
#include "constant_medium.h"
#include "rect.h"

scene_t random_scene(int image_width, int image_height) {
    
    constexpr point3 look_from(13,2,3);
    constexpr point3 look_at(0,0,0);
    constexpr dvec3_t vup(0,1,0);
    constexpr auto dist_to_focus = 10.0;
    constexpr auto aperture = 0.1;

    camera_t cam(
        image_width, 
        image_height, 
        60.0, 
        look_from, 
        look_at, 
        vup,
        aperture,
        dist_to_focus,
        0.0,
        1.0
        );

    scene_t scene {cam};

    //auto ground_material = make_shared<lambertian_material_t>(color_t(0.5, 0.5, 0.5));
    auto ground_material = make_shared<lambertian_material_t>(make_shared<checker_texture_t>(color_t(0, 0, 0), color_t(0.8, 0.8, 0.8)));
    scene.entities.add(make_shared<sphere_t>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            const auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material_t> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color_t::random() * color_t::random();
                    sphere_material = make_shared<lambertian_material_t>(albedo);
                    auto center2 = center + dvec3_t(0, random_double(0, 0.5), 0);
                    scene.entities.add(make_shared<sphere_t>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal_material_t
                    auto albedo = color_t::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal_material_t>(albedo, fuzz);
                    scene.entities.add(make_shared<sphere_t>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric_material_t>(1.5);
                    scene.entities.add(make_shared<sphere_t>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric_material_t>(1.5);
    scene.entities.add(make_shared<sphere_t>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian_material_t>(color_t(0.4, 0.2, 0.1));
    scene.entities.add(make_shared<sphere_t>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal_material_t>(color_t(0.7, 0.6, 0.5), 0.0);
    scene.entities.add(make_shared<sphere_t>(point3(4, 1, 0), 1.0, material3));

    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    scene.background = {0.70, 0.80, 1.00};
    return scene;
}

scene_t three_spheres_scene(int image_width, int image_height) {
    
    constexpr point3 look_from(3,3,2);
    constexpr point3 look_at(0,0,-1);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 1.0;

    camera_t cam {image_width, image_height, 60.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};


    auto material_ground = make_shared<lambertian_material_t>(color_t(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian_material_t>(color_t(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric_material_t>(1.5);
    auto material_right  = make_shared<metal_material_t>(color_t(0.8, 0.6, 0.2), 0.0);

    scene.entities.add(make_shared<sphere_t>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    scene.entities.add(make_shared<sphere_t>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    scene.entities.add(make_shared<sphere_t>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    scene.entities.add(make_shared<sphere_t>(point3(-1.0,    0.0, -1.0), -0.45, material_left));
    scene.entities.add(make_shared<sphere_t>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

    scene.background = {0.70, 0.80, 1.00};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;
}

scene_t earth_scene(int image_width, int image_height) {
    constexpr point3 look_from(13,2,3);
    constexpr point3 look_at(0,0, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 1.0;

    camera_t cam {image_width, image_height, 20.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};

    auto earth_texture = make_shared<image_texture_t>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian_material_t>(earth_texture);
    auto globe = make_shared<sphere_t>(point3(0,0,0), 2, earth_surface);

    scene.entities.add(globe);
    scene.background = {0.70, 0.80, 1.00};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;
}

scene_t two_perlin_spheres_scene(int image_width, int image_height) {
    constexpr point3 look_from(13,2,3);
    constexpr point3 look_at(0,0, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    camera_t cam {image_width, image_height, 20.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};

    auto pertext = make_shared<noise_texture_t>(4);
    scene.entities.add(make_shared<sphere_t>(point3(0,-1000,0), 1000, make_shared<lambertian_material_t>(pertext)));
    scene.entities.add(make_shared<sphere_t>(point3(0, 2, 0), 2, make_shared<lambertian_material_t>(pertext)));

    scene.background = {0.70, 0.80, 1.00};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;
}

scene_t simple_light(int image_width, int image_height) {
    constexpr point3 look_from(0,3,20);
    constexpr point3 look_at(0,2, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    camera_t cam {image_width, image_height, 20.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};

    auto light_rotation = glm::angleAxis<float, glm::qualifier::defaultp>((float)degrees_to_radians(-90.0), {0.0f, 1.0f, 0.0f});

    auto pertext = make_shared<noise_texture_t>(4);
    scene.entities.add(make_shared<sphere_t>(point3(0,-1000,0), 1000, make_shared<lambertian_material_t>(pertext)));
    scene.entities.add(make_shared<sphere_t>(point3(0, 2, 0), 2, make_shared<lambertian_material_t>(pertext)));

    auto difflight = make_shared<diffuse_light>(color_t(4,4,4));

    scene.entities.add(make_shared<rect_t>(3, 3, dvec3_t{3.0, 2.0f, 0}, light_rotation, difflight));
    

    scene.background = {0.01, 0.02, 0.03};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;    
}

scene_t simple_box(int image_width, int image_height) {
    constexpr point3 look_from(15,15,20);
    constexpr point3 look_at(0,2, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    camera_t cam {image_width, image_height, 20.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};
    auto box_rotation = glm::angleAxis<float, glm::qualifier::defaultp>((float)degrees_to_radians(-45.0), {0.0f, 1.0f, 0.0f});
    auto ground_material = make_shared<lambertian_material_t>(make_shared<checker_texture_t>(color_t(0, 0, 0), color_t(0.8, 0.8, 0.8)));
    auto pertext = make_shared<noise_texture_t>(4);
    scene.entities.add(make_shared<sphere_t>(point3(0,-1000,0), 1000, ground_material));
    scene.entities.add(make_shared<box_t>(point3(0, 2, 0), box_rotation, 4, 0.5, 2, make_shared<lambertian_material_t>(pertext)));
    

    scene.background = {0.70, 0.80, 1.00};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;    
}

scene_t box_test(int image_width, int image_height) {
    constexpr point3 look_from(15,15,20);
    constexpr point3 look_at(0,2, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    camera_t cam {image_width, image_height, 20.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};
    auto box_rotation = glm::angleAxis<float, glm::qualifier::defaultp>((float)degrees_to_radians(-45.0), {0.0f, 1.0f, 0.0f});
    auto ground_material = make_shared<lambertian_material_t>(make_shared<checker_texture_t>(color_t(0, 0, 0), color_t(0.8, 0.8, 0.8)));
    auto pertext = make_shared<noise_texture_t>(4);
    scene.entities.add(make_shared<sphere_t>(point3(0,-1000,0), 1000, ground_material));
    scene.entities.add(make_shared<box_t>(point3(0, 2, 0), box_rotation, 4, 0.5, 2, make_shared<lambertian_material_t>(pertext)));
    

    scene.background = {0.70, 0.80, 1.00};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;    
}



scene_t cornell_box(int image_width, int image_height) {
    constexpr point3 look_from(500,500,800);
    constexpr point3 look_at(0,0, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    camera_t cam {image_width, image_height, 40.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};

    auto red   = make_shared<lambertian_material_t>(color_t(.65, .05, .05));
    auto white = make_shared<lambertian_material_t>(color_t(.73, .73, .73));
    auto green = make_shared<lambertian_material_t>(color_t(.12, .45, .15));
    auto blue = make_shared<lambertian_material_t>(color_t(.12, .45, .85));
    auto light = make_shared<diffuse_light>(color_t(15, 15, 15));
    
    auto r90y = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(90.0), {0.0, 1.0, 0.0});
    auto r90x = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(90.0), {1.0, 0.0, 0.0});

    auto r15y = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(15.0), {0.0, 1.0, 0.0});
    auto rn15y = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(-15.0), {0.0, 1.0, 0.0});

    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{0, 0, -555.5}, glm::quat(), white));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{0, -277.5, -277.5}, r90x, white));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{0,  277.5, -277.5}, r90x, white));
    scene.entities.add(make_shared<rect_t>(150, 150, dvec3_t{0,  277, -277.5}, r90x, light));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{-277.5, 0, -277.5}, r90y, red));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{ 277.5, 0, -277.5}, r90y, green));

    //add some boxes
    scene.entities.add(make_shared<box_t>(dvec3_t{50, -277.5 + (150.0/2.0), -150}, rn15y, 150, 150, 150, white));
    scene.entities.add(make_shared<box_t>(dvec3_t{-100, -277.5 + (250/2.0), -400}, r15y, 200, 250, 200, blue));
    

    scene.background = {0.0, 0.0, 0.0};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;    
}

scene_t cornell_smoke_box(int image_width, int image_height) {
    constexpr point3 look_from(0,0,800);
    constexpr point3 look_at(0,0, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    camera_t cam {image_width, image_height, 40.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};

    auto red   = make_shared<lambertian_material_t>(color_t(.65, .05, .05));
    auto white = make_shared<lambertian_material_t>(color_t(.73, .73, .73));
    auto green = make_shared<lambertian_material_t>(color_t(.12, .45, .15));
    auto blue = make_shared<lambertian_material_t>(color_t(.12, .45, .85));
    auto light = make_shared<diffuse_light>(color_t(15, 15, 15));
    
    auto r90y = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(90.0), {0.0, 1.0, 0.0});
    auto r90x = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(90.0), {1.0, 0.0, 0.0});

    auto r15y = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(15.0), {0.0, 1.0, 0.0});
    auto rn15y = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(-15.0), {0.0, 1.0, 0.0});

    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{0, 0, -555.5}, glm::quat(), white));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{0, -277.5, -277.5}, r90x, white));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{0,  277.5, -277.5}, r90x, white));
    scene.entities.add(make_shared<rect_t>(150, 150, dvec3_t{0,  277, -277.5}, r90x, light));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{-277.5, 0, -277.5}, r90y, red));
    scene.entities.add(make_shared<rect_t>(555, 555, dvec3_t{ 277.5, 0, -277.5}, r90y, green));

    //add some boxes
    auto box1 = make_shared<box_t>(dvec3_t{0, 0, 0}, glm::quat(), 150, 150, 150, white);
    //scene.entities.add(box1);
    scene.entities.add(make_shared<constant_medium_t>(box1, 0.01, color_t(0,0,0)));
    

    scene.background = {0.0, 0.0, 0.0};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;    
}


scene_t all_test(int image_width, int image_height) {

    constexpr point3 look_from(478,278,-600);
    constexpr point3 look_at(278,278, 0);
    constexpr dvec3_t vup(0,1,0);
    const auto dist_to_focus = glm::length(look_from-look_at);
    constexpr auto aperture = 0.1;

    auto r90x = glm::angleAxis<double, glm::qualifier::defaultp>(degrees_to_radians(90.0), {1.0, 0.0, 0.0});
    camera_t cam {image_width, image_height, 40.0, look_from, look_at, vup, aperture, dist_to_focus};
        
    scene_t scene {cam};
    
    auto ground = make_shared<lambertian_material_t>(color_t(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0;
            
            scene.entities.add(make_shared<box_t>(point3(x0,y0,z0), glm::dquat(), 100,random_double(10, 150), 100, ground));
        }
    }

    scene.entities.add(make_shared<sphere_t>(point3(260, 150, 45), 50, make_shared<dielectric_material_t>(1.5)));
    scene.entities.add(make_shared<sphere_t>(
        point3(0, 150, 145), 50, make_shared<metal_material_t>(color_t(0.8, 0.8, 0.9), 1.0)
    ));

    auto light = make_shared<diffuse_light>(color_t(7, 7, 7));
    scene.entities.add(make_shared<rect_t>(300, 300, dvec3_t{0, 554, 0}, r90x, light));

    scene.background = {0.0, 0.0, 0.0};
    scene.root = std::make_shared<bvh_node_t>(scene.entities, 0, 1);
    return scene;
}
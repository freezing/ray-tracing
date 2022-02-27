#include <iostream>
#include <assert.h>
#include <math.h>
#include <vector>
#include <tuple>
#include <variant>
#include <optional>

#include "camera.h"
#include "common.h"
#include "material.h"
#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "ppm.h"
#include "hit_record.h"
#include "engine.h"
#include "renderer.h"

Material choose_material() {
      double random_sample = random_double();
      if (random_sample < 0.8) {
        // Diffuse.
        Vec3 albedo = random_unit_vector() * random_unit_vector();
        return LambertianMaterial{albedo};
      } else if (random_sample < 0.95) {
        // Metal.
        Vec3 albedo = random_vector(0.5, 1);
        double fuzz = random_double(0, 0.5);
        return MetalMaterial{albedo, fuzz};
      } else {
        // Glass.
        return DielectricMaterial{.refraction_index = 1.5};
      }
}

World random_world() {
  World world;

  // Add ground (a very large sphere).
  Material ground_material = LambertianMaterial{Vec3(0.5, 0.5, 0.5)};
  world.add(Sphere(Vec3(0, -1000, 0), 1000), ground_material);

  for (int x = -11; x < 11; x++) {
    for (int z = -11; z < 11; z++) {
      Vec3 center{x + 0.9 * random_double(), 0.2, z + 0.9 * random_double()};
      double radius = 0.2;

      if ((center - Vec3{4, 0.2, 0}).length() > 0.9) {
        Material material = choose_material();
        world.add(Sphere(std::move(center), radius), material);
      }
    }
  }

  world.add(Sphere({0, 1, 0}, 1.0), DielectricMaterial{1.5});
  world.add(Sphere({-4, 1, 0}, 1.0), LambertianMaterial{Vec3{0.4, 0.2, 0.1}});
  world.add(Sphere({4, 1, 0}, 1.0), MetalMaterial{{0.7, 0.6, 0.5}, 0.0});

  return world;
}

int main(int argc, char** argv) {
  // Image
  const double aspect_ratio = 16.0 / 9.0;
  const int image_width = 1920;
  const int image_height = static_cast<int>(image_width / aspect_ratio);
  const int samples_per_pixel = 500;
  const int max_ray_bounce_depth = 50;

  std::cerr << "Image size: " << image_width << ", " << image_height << std::endl;

  // Camera
  constexpr Vec3 origin{13, 2, 3};
  constexpr Vec3 look_at{0, 0, 0};
  constexpr Vec3 view_up{0, 1, 0};
  constexpr double vertical_field_of_view = 45.0;
  constexpr double aperture = 0.1;
  const double focus_distance = 10.0;
  const Camera camera(origin, look_at, view_up, vertical_field_of_view, aspect_ratio, aperture, focus_distance);

  // Materials
  constexpr Vec3 center_color(0.7, 0.3, 0.3);
  constexpr Vec3 left_color(0.8, 0.8, 0.8);
  constexpr Vec3 right_color(0.8, 0.6, 0.2);

  constexpr Material material_ground = LambertianMaterial{ground_color};
  constexpr Material material_center = LambertianMaterial{center_color};
  constexpr Material material_left = DielectricMaterial{1.5};
  constexpr Material material_right = MetalMaterial{right_color, 0.5};
  constexpr Material lambertian_blue = LambertianMaterial{pure_blue_color};
  constexpr Material lambertian_red = LambertianMaterial{pure_red_color};

  // World
  World world;
  world.add(Sphere({0, -100.5, -1}, 100), material_ground);
  world.add(Sphere({0, 0, -1}, 0.5), material_center);
  world.add(Sphere({-1, 0, -1}, 0.5), material_left);
  world.add(Sphere({-1, -0, -1}, -0.4), material_left);
  world.add(Sphere({1, 0, -1}, 0.5), material_right);

  // Another world
  const double R = cos(PI / 4.0);
  World another_world;
  another_world.add(Sphere({-R, 0, -1}, R), lambertian_blue);
  another_world.add(Sphere({R, 0, -1}, R), lambertian_red);

  // Random big world
  World big_world = random_world();

  // Render
  std::cout << "P3" << std::endl << image_width << ' ' << image_height << std::endl << 255 << std::endl;

  Renderer renderer{world, camera, image_width, image_height, samples_per_pixel, max_ray_bounce_depth};

  for (int row = image_height - 1; row >= 0; row--) {
    std::cerr << "\rScanlines remaining: " << (row) << ' ' << std::flush;
    for (int col = 0; col < image_width; col++) {
      Vec3 pixel_color = renderer.color_at(row, col);
      write_pixel(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone.\n";
  return 0;
}
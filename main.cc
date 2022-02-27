#include <iostream>
#include <assert.h>
#include <mutex>
#include <map>
#include <assert.h>
#include <math.h>
#include <vector>
#include <tuple>
#include <variant>
#include <optional>
#include <thread>

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
#include "task_splitter.h"
#include "task_renderer.h"

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
  Renderer renderer{world, camera, image_width, image_height, samples_per_pixel, max_ray_bounce_depth};

  struct RenderedImage {
    std::map<PixelLocation, Vec3> pixels;

    void add(const RenderedImage& image) {
      for (const auto& entry : image.pixels) {
        pixels.insert(entry);
      }
    }
  };

  const int num_cores = 32;
  auto tasks = split_tasks(image_height, image_width, num_cores);
  std::cerr << "Number of cores:" << ' ' << num_cores << std::endl << std::flush;

  for (const auto& tasks_per_core : tasks) {
    std::cerr << "  Num tasks[" << tasks_per_core.core_id << "]: " << tasks_per_core.tasks.size() << std::endl << std::flush;
  }

  std::vector<RenderedImage> rendered_images;
  rendered_images.resize(num_cores);

  std::vector<std::thread> threads;
  for (const auto& tasks_per_core : tasks) {
    std::thread thread([&renderer, &tasks_per_core, &rendered_images]() mutable {
      auto& rendered_image = rendered_images[tasks_per_core.core_id];
      for (const auto& task : tasks_per_core.tasks) {
        auto result = render_task(tasks_per_core.core_id, task, renderer, tasks_per_core.core_id == 0);
        // Populate result in the rendered_image map.
        for (std::size_t i = 0; i < result.pixels.size(); i++) {
          auto location = result.location_of(i);
          rendered_image.pixels[location] = result.pixels[i];
        }
      }
    });
    threads.push_back(std::move(thread));
  }

  // Merge image parts into one image.
  for (auto& thread : threads) {
    thread.join();
  }

  RenderedImage rendered_image = std::move(rendered_images[0]);
  for (std::size_t i = 1; i < rendered_images.size(); i++) {
    rendered_image.add(rendered_images[i]);
  }

  // Save in file
  std::cout << "P3" << std::endl << image_width << ' ' << image_height << std::endl << 255 << std::endl;

  for (int row = image_height - 1; row >= 0; row--) {
    std::cerr << "\rScanlines remaining: " << (row) << ' ' << std::endl << std::flush;
    for (int col = 0; col < image_width; col++) {
      PixelLocation location{col, row};
      auto it = rendered_image.pixels.find(location);
      if (it != rendered_image.pixels.end()) {
        Vec3 pixel_color = it->second;
        write_pixel(std::cout, pixel_color);
      } else {
        assert(false);
        std::cerr << "Missing pixel at location: " << to_debug(location) << std::endl;
      }
    }
  }

  std::cerr << "\nDone.\n";
  return 0;
}
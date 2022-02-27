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

using Object = std::variant<Sphere>;

struct HitVisitorFn {
  const Ray& ray;
  const Material& material;

  std::tuple<Vec3, bool> calculate_normal_and_front_face(const Vec3& outward_normal) {
    if (dot(ray.direction(), outward_normal) > 0.0) {
      return {-outward_normal, false};
    } else {
      return {outward_normal, true};
    }
  }

  std::optional<HitRecord> operator() (const Sphere& sphere) {
    Vec3 d_center = ray.origin() - sphere.center();
    double a = ray.direction().length_squared();
    double b = 2.0 * dot(d_center, ray.direction());
    double c = d_center.length_squared() - sqr(sphere.radius());
    double discriminant = b * b - 4 * a * c;
    if (discriminant <= 0) {
      return {};
    } else {
      double t = (-b - sqrt(discriminant)) / (2.0 * a);
      Vec3 point = ray.at(t);
      Vec3 outward_normal = unit_vector(point - sphere.center());
      auto [normal, front_face] = calculate_normal_and_front_face(outward_normal);      
      return {{point, normal, t, front_face, material}};
    }
  }
};

std::optional<HitRecord> hit_object(const Ray& ray, const Object& object, const Material& material) {
  return std::visit(HitVisitorFn{ray, material}, object);
}

class World {
public:
  World() {}
  
  void clear() {
    objects_.clear();
  }

  void add(Object&& object, Material material) {
    objects_.push_back({std::move(object), std::move(material)});
  }

  std::optional<HitRecord> hit(const Ray& ray, double t_min, double t_max) const {
    std::optional<HitRecord> closest_hit;
    for (const auto& [object, material] : objects_) {
      std::optional<HitRecord> hit_record = hit_object(ray, object, material);
      double upper_bound = closest_hit ? closest_hit->t : t_max;
      if (hit_record && is_within_bounds(hit_record->t, t_min, upper_bound)) {
        closest_hit = hit_record;
      }
    }
    assert(!closest_hit || is_within_bounds(closest_hit->t, t_min, t_max));
    return closest_hit;
  }


private:
  std::vector<std::tuple<Object, Material>> objects_;

  static bool is_within_bounds(double x, double min, double max) {
    return min <= x && x <= max;
  }

};

Vec3 ray_color(const Ray& ray, const World& world, int depth) {
  if (depth <= 0) {
    // No more light is gathered if the ray bounce limit is exceeded.
    return black_color;
  }

  std::optional<HitRecord> hit_record = world.hit(ray, 0.001, INFINITY);

  if (hit_record) {
    auto scattered_ray = std::visit(ScatterMaterialFn{ray, as_scatter_info(*hit_record)}, hit_record->material);
    if (!scattered_ray) {
      return black_color;
    }
    return scattered_ray->attenuation_color * ray_color(scattered_ray->ray, world, depth - 1);
  } else {
    Vec3 unit_direction = unit_vector(ray.direction());
    double t = 0.5 * (unit_direction.y() + 1.0);
    return lerp_vector(t, white_color, blue_color);
  }
}

int main(int argc, char** argv) {
  // Image
  const double aspect_ratio = 16.0 / 9.0;
  const int image_width = 1920;
  const int image_height = static_cast<int>(image_width / aspect_ratio);
  const int samples_per_pixel = 100;
  const int max_ray_bounce_depth = 50;

  std::cerr << "Image size: " << image_width << ", " << image_height << std::endl;

  // Camera
  constexpr Vec3 origin{3, 3, 2};
  constexpr Vec3 look_at{0, 0, -1};
  constexpr Vec3 view_up{0, 1, 0};
  constexpr double vertical_field_of_view = 45.0;
  constexpr double aperture = 2.0;
  const double focus_distance = (origin - look_at).length();
  Camera camera(origin, look_at, view_up, vertical_field_of_view, aspect_ratio, aperture, focus_distance);

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

  // Render
  std::cout << "P3" << std::endl << image_width << ' ' << image_height << std::endl << 255 << std::endl;

  for (int row = image_height - 1; row >= 0; row--) {
    std::cerr << "\rScanlines remaining: " << (row) << ' ' << std::flush;
    for (int col = 0; col < image_width; col++) {
      Vec3 pixel_color{0, 0, 0};
      for (int s = 0; s < samples_per_pixel; s++) {
        double u = (double(col) + random_double()) / (image_width - 1);
        double v = (double(row) + random_double()) / (image_height - 1);
        Ray ray = camera.ray_at(u, v);
        pixel_color += ray_color(ray, world, max_ray_bounce_depth);
      }
      pixel_color /= samples_per_pixel;
      write_pixel(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone.\n";
  return 0;
}
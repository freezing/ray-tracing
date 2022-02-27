#pragma once

#include <variant>
#include <optional>
#include <tuple>

#include "ray.h"
#include "vec3.h"
#include "sphere.h"
#include "hit_record.h"
#include "world.h"

namespace detail {
  bool is_within_bounds(double x, double min, double max) {
    return min <= x && x <= max;
  }

  std::tuple<Vec3, bool> calculate_normal_and_front_face(const Ray& ray, const Vec3& outward_normal) {
    if (dot(ray.direction(), outward_normal) > 0.0) {
      return {-outward_normal, false};
    } else {
      return {outward_normal, true};
    }
  }

  std::optional<HitRecord> hit_sphere(const Ray& ray, const Material& material, const Sphere& sphere) {
      Vec3 d_center = ray.origin() - sphere.center();
      double a = ray.direction().length_squared();
      double half_b = dot(d_center, ray.direction());
      double c = d_center.length_squared() - sqr(sphere.radius());
      double discriminant = half_b * half_b - a * c;
      if (discriminant <= 0) {
        return {};
      } else {
        double t = (-half_b - sqrt(discriminant)) / a;
        Vec3 point = ray.at(t);
        Vec3 outward_normal = unit_vector(point - sphere.center());
        auto [normal, front_face] = calculate_normal_and_front_face(ray, outward_normal);      
        return {{point, normal, t, front_face, material}};
      }
  }

  struct HitVisitorFn {
    const Ray& ray;
    const Material& material;

    std::optional<HitRecord> operator() (const Sphere& sphere) {
      return hit_sphere(ray, material, sphere);
    }
  };

  std::optional<HitRecord> hit_object(const Ray& ray, const Object& object, const Material& material) {
    return std::visit(HitVisitorFn{ray, material}, object);
  }
}

class Engine {
public:
  Vec3 ray_color(const Ray& ray, const World& world, int depth) {
    if (depth <= 0) {
      // No more light is gathered if the ray bounce limit is exceeded.
      return black_color;
    }

    Engine engine{};
    std::optional<HitRecord> hit_record = engine.hit_world(world, ray, 0.001, INFINITY);

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

private:
  std::optional<HitRecord> hit_world(const World& world, const Ray& ray, double t_min, double t_max) {
    std::optional<HitRecord> closest_hit;
    for (const auto& [object, material] : world.objects()) {
      std::optional<HitRecord> hit_record = detail::hit_object(ray, object, material);
      double upper_bound = closest_hit ? closest_hit->t : t_max;
      if (hit_record && detail::is_within_bounds(hit_record->t, t_min, upper_bound)) {
        closest_hit = hit_record;
      }
    }
    assert(!closest_hit || detail::is_within_bounds(closest_hit->t, t_min, t_max));
    return closest_hit;
  }
};
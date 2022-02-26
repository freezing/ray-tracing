#pragma once

#include <optional>

#include "vec3.h"
#include "ray.h"

struct ScatterInfo {
    Vec3 point;
    Vec3 normal;
};

struct ScatteredRay {
    Ray ray;
    Vec3 attenuation_color;
};

Vec3 reflect(const Vec3& v, const Vec3& normal) {
    return v - 2 * dot(v, normal) * normal;
}

struct LambertianMaterial {
    Vec3 albedo;

    std::optional<ScatteredRay> scatter(const Ray& ray, const ScatterInfo& scatter_info) const {
        Vec3 scatter_direction = scatter_info.normal + random_unit_vector();

        // Catch degenerate scatter direction.
        if (scatter_direction.is_near_zero()) {
            scatter_direction = scatter_info.normal;
        }

        Ray scattered_ray{scatter_info.point, scatter_direction};
        Vec3 attenuation_color = albedo;
        return {{scattered_ray, attenuation_color}};
    }
};

struct MetalMaterial {
    Vec3 albedo;
    double fuzz;

    std::optional<ScatteredRay> scatter(const Ray& ray, const ScatterInfo& scatter_info) const {
        Vec3 reflected = reflect(unit_vector(ray.direction()), scatter_info.normal);
        Vec3 direction = reflected + fuzz * random_in_unit_sphere();
        Ray scattered_ray{scatter_info.point, direction};
        Vec3 attenuation_color = albedo;
        return {{scattered_ray, attenuation_color}};
    }
};

using Material = std::variant<LambertianMaterial, MetalMaterial>;

struct ScatterMaterialFn {
    const Ray& ray;
    const ScatterInfo& scatter_info;

    std::optional<ScatteredRay> operator () (const LambertianMaterial& material) const {
        return material.scatter(ray, scatter_info);
    }

    std::optional<ScatteredRay> operator () (const MetalMaterial& material) const {
        return material.scatter(ray, scatter_info);
    }
};

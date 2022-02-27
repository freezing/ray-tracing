#pragma once

#include <optional>

#include "vec3.h"
#include "ray.h"

struct ScatterInfo {
    Vec3 point;
    Vec3 normal;
    bool front_face;
};

struct ScatteredRay {
    Ray ray;
    Vec3 attenuation_color;
};

Vec3 reflect(const Vec3& v, const Vec3& normal) {
    return v - 2 * dot(v, normal) * normal;
}

Vec3 refract(const Vec3& v, const Vec3& normal, double etai_over_etat) {
    double cos_theta = fmin(dot(-v, normal), 1.0);
    Vec3 r_out_perp = etai_over_etat * (v + cos_theta * normal);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * normal;
    return r_out_perp + r_out_parallel;
}

double reflectance(double cosine, double refraction_ratio) {
    // Use Schlick's approcimation for reflectance.
    double r0 = (1.0 - refraction_ratio) / (1.0 + refraction_ratio);
    double r = r0 * r0;
    return r + (1.0 - r) * pow(1 - cosine, 5);
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

struct DielectricMaterial {
    double refraction_index;

    std::optional<ScatteredRay> scatter(const Ray& ray, const ScatterInfo& scatter_info) const {
        double refraction_ratio = scatter_info.front_face ? (1.0 / refraction_index) : refraction_index;
        Vec3 attenuation = white_color;
        Vec3 unit_direction = unit_vector(ray.direction());
        double cos_theta = fmin(dot(-unit_direction, scatter_info.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        // If there is no solution to the refraction equation.
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        bool should_reflect = reflectance(cos_theta, refraction_index) > random_double();
        
        Vec3 direction;
        if (cannot_refract || should_reflect) {
            direction = reflect(unit_direction, scatter_info.normal);
        } else {
            direction = refract(unit_direction, scatter_info.normal, refraction_ratio);
        }
        Ray scattered_ray{scatter_info.point, direction};
        return {{scattered_ray, attenuation}};
    }
};

using Material = std::variant<LambertianMaterial, MetalMaterial, DielectricMaterial>;

struct ScatterMaterialFn {
    const Ray& ray;
    const ScatterInfo& scatter_info;

    std::optional<ScatteredRay> operator () (const LambertianMaterial& material) const {
        return material.scatter(ray, scatter_info);
    }

    std::optional<ScatteredRay> operator () (const MetalMaterial& material) const {
        return material.scatter(ray, scatter_info);
    }

    std::optional<ScatteredRay> operator () (const DielectricMaterial& material) const {
        return material.scatter(ray, scatter_info);
    }
};

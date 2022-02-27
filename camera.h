#pragma once

#include "common.h"
#include "vec3.h"
#include "ray.h"

class Camera {
public:
    Camera(
            Vec3 origin, 
            Vec3 look_at, 
            Vec3 view_up, 
            double vertical_fov_degrees, 
            double aspect_ratio,
            double aperture, 
            double focus_distance) {
        double theta = degrees_to_radians(vertical_fov_degrees);
        double h = tan(theta / 2.0);
        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        Vec3 w = unit_vector(origin - look_at);
        Vec3 u = unit_vector(cross(view_up, w));
        Vec3 v = cross(w, u);

        Vec3 horizontal = focus_distance * viewport_width * u;
        Vec3 vertical = focus_distance * viewport_height * v;
        Vec3 lower_left_corner = origin - horizontal/2 - vertical/2 - focus_distance * w;

        // Set fields.
        origin_ = origin;
        lower_left_corner_ = lower_left_corner;
        horizontal_ = horizontal;
        vertical_ = vertical;
        u_ = u;
        v_ = v;
        w_ = w;
        lens_radius_ = aperture / 2;
    }

    Ray ray_at(double s, double t) const {
        Vec3 random = lens_radius_ * random_in_unit_disk();
        Vec3 offset = u_ * random.x() + v_ * random.y();
        Vec3 ray_origin = origin_ + offset;
        Vec3 direction = lower_left_corner_ + s * horizontal_ + t * vertical_ - ray_origin;
        return Ray{ray_origin, direction};
    }

    const Vec3& origin() const {
        return origin_;
    }

    const Vec3& lower_left_corner() const {
        return lower_left_corner_;
    }

    const Vec3& horizontal() const {
        return horizontal_;
    }

    const Vec3& vertical() const {
        return vertical_;
    }

private:
    Vec3 origin_;
    Vec3 lower_left_corner_;
    Vec3 horizontal_;
    Vec3 vertical_;
    Vec3 u_;
    Vec3 v_;
    Vec3 w_;
    double lens_radius_;
};
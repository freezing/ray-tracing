#pragma once

#include "common.h"
#include "vec3.h"
#include "ray.h"

class Camera {
public:
    Camera(Vec3 origin, Vec3 look_at, Vec3 view_up, double vertical_fov_degrees, double aspect_ratio) {
        double theta = degrees_to_radians(vertical_fov_degrees);
        double h = tan(theta / 2.0);
        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        Vec3 w = unit_vector(origin - look_at);
        Vec3 u = unit_vector(cross(view_up, w));
        Vec3 v = cross(w, u);

        horizontal_ = viewport_width * u;
        vertical_ = viewport_height * v;
        origin_ = origin;
        lower_left_corner_ = origin_ - horizontal_/2 - vertical_/2 - w;
    }

    Ray ray_at(double u, double v) const {
        Vec3 direction = lower_left_corner_ + u * horizontal_ + v * vertical_ - origin_;
        return Ray{origin_, std::move(direction)};
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
};
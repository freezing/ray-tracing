#pragma once

#include "vec3.h"
#include "ray.h"

class Camera {
public:
    Camera() {
        double aspect_ratio = 16.0 / 9.0;
        double viewport_height = 2.0;
        double viewport_width = aspect_ratio * viewport_height;
        double focal_length = 1.0;
        Vec3 focal_distance{0, 0, focal_length};
        origin_ = Vec3(0, 0, 0);
        horizontal_ = Vec3(viewport_width, 0.0, 0.0);
        vertical_ = Vec3(0.0, viewport_height, 0.0);
        lower_left_corner_ = origin_ - horizontal_/2 - vertical_/2 - focal_distance;
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
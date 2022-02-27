#pragma once

#include "common.h"
#include "camera.h"
#include "ray.h"
#include "vec3.h"
#include "engine.h"

class Renderer {
public:
    Renderer(
        const World& world,
        const Camera& camera,
        int image_width,
        int image_height,
        int samples_per_pixel, 
        int max_ray_bounce_depth) 
    : world_{world}
    , camera_{camera}
    , image_width_{image_width}
    , image_height_{image_height}
    , samples_per_pixel_{samples_per_pixel}
    , max_ray_bounce_depth_{max_ray_bounce_depth} {}

    Vec3 color_at(int row, int col) const {
        Engine engine{};    
        Vec3 pixel_color{0, 0, 0};
        for (int s = 0; s < samples_per_pixel_; s++) {
            double u = (double(col) + random_double()) / (image_width_ - 1);
            double v = (double(row) + random_double()) / (image_height_ - 1);
            Ray ray = camera_.ray_at(u, v);
            pixel_color += engine.ray_color(ray, world_, max_ray_bounce_depth_);
        }
        pixel_color /= samples_per_pixel_;
        return pixel_color;
    }

private:
    const World& world_;
    const Camera& camera_;
    int image_width_;
    int image_height_;
    int samples_per_pixel_;
    int max_ray_bounce_depth_;
};
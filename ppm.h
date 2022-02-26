#pragma once

#include "vec3.h"
#include "common.h"

double gamma_correct(double value, double gamma = 2.0) {
    return pow(value, 1.0 / gamma);
}

void write_pixel(std::ostream& out, const Vec3& pixel) {
    int r = static_cast<int>(256 * clamp(gamma_correct(pixel[0]), 0.0, 0.999));
    int g = static_cast<int>(256 * clamp(gamma_correct(pixel[1]), 0.0, 0.999));
    int b = static_cast<int>(256 * clamp(gamma_correct(pixel[2]), 0.0, 0.999));
    out << r << ' ' << g << ' ' << b << std::endl;
}
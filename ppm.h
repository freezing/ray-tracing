#pragma once

#include "vec3.h"
#include "common.h"

void write_pixel(std::ostream& out, const Vec3& pixel) {
      int r = clamp(static_cast<int>(255.999 * pixel[0]), 0, 255);
      int g = clamp(static_cast<int>(255.999 * pixel[1]), 0, 255);
      int b = clamp(static_cast<int>(255.999 * pixel[2]), 0, 255);
      out << r << ' ' << g << ' ' << b << std::endl;
}
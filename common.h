#pragma once

constexpr double POSITIVE_INFINITY = std::numeric_limits<double>::infinity();
constexpr double PI = M_PI;

inline double degrees_to_radians(double degrees) {
  return degrees * PI / 180.0;
}

inline double random_double() {
  // Returns a random real in [0, 1).
  return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
  // Returns a random real in [min, max).
  return min + (max - min) * random_double();
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}
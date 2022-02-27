#pragma once

constexpr double POSITIVE_INFINITY = std::numeric_limits<double>::infinity();
constexpr double PI = M_PI;
constexpr Vec3 black_color(0, 0, 0);
constexpr Vec3 white_color(1.0, 1.0, 1.0);
constexpr Vec3 blue_color(0.5, 0.7, 1.0);
constexpr Vec3 green_color(0.5, 1.0, 0.7);
constexpr Vec3 ground_color(0.8, 0.8, 0.0);

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

inline Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 u{random_double(), random_double(), random_double()};
        if (u.length_squared() <= 1.0) {
            return u;
        }
    }
}

inline Vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

inline Vec3 random_in_hemisphere(const Vec3& normal) {
    Vec3 in_unit_sphere = random_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.0)  {
        // In the same hemisphere as the normal.
        return in_unit_sphere;
    } else {
        return -in_unit_sphere;
    }
}
#pragma once

#include "vec3.h"

class Sphere {
public:
    // constexpr Sphere(const Vec3& center, double radius) : center_{center}, radius_{radius} {}
    constexpr Sphere(Vec3&& center, double radius) : center_{center}, radius_{radius} {}

    const Vec3& center() const {
        return center_;
    }

    const double radius() const {
        return radius_;
    }

private: 
    Vec3 center_;
    double radius_;
};
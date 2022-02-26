#pragma once

#include "vec3.h"

class Ray {
public:
    Ray() {}
    Ray(const Vec3& origin, const Vec3& direction) : origin_{origin}, direction_{direction} {}
    Ray(Vec3&& origin, Vec3&& direction) : origin_{std::move(origin)}, direction_{std::move(direction)} {}

    const Vec3& origin() const { return origin_; }

    const Vec3& direction() const { return direction_; }

    Vec3 at(double t) const {
        return origin_ + t * direction_;
    }

private:
    Vec3 origin_;
    Vec3 direction_;
};
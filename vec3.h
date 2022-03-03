#pragma once

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>

namespace {
    static constexpr double sqr(double x) {
        return x * x;
    }
}

class Vec3 {
public:
    constexpr Vec3() : e{0, 0, 0} {}
    constexpr Vec3(const Vec3& v) : e{v[0], v[1], v[2]} {}
    constexpr Vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    constexpr double x() const { return e[0]; }
    constexpr double y() const { return e[1]; }
    constexpr double z() const { return e[2]; }

    constexpr Vec3 operator-() const { 
        return Vec3(-e[0], -e[1], -e[2]); 
    }

    constexpr double operator[](int i) const { 
        return e[i]; 
    }

    constexpr double& operator[](int i) { 
        return e[i]; 
    }

    constexpr Vec3& operator +=(const Vec3& v) {
        e[0] += v[0];
        e[1] += v[1];
        e[2] += v[2];
        return *this;
    }

    constexpr Vec3& operator *=(const double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    constexpr Vec3& operator /=(const double t) {
        return (*this) *= 1.0 / t;
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    constexpr double length_squared() const {
        return sqr(e[0]) + sqr(e[1]) + sqr(e[2]);
    }

    constexpr bool is_near_zero(double eps = 0.001) const {
        return abs(e[0]) < eps && abs(e[1]) < eps && abs(e[2]) < eps;
    }


private:
    double e[3];
};

// TODO: I want to have a proper serializer, not just assume the serialization format.
// inline std::ostream& operator << (std::ostream&)

inline Vec3 operator + (const Vec3& lhs, const Vec3& rhs) {
    return Vec3(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]);
}

inline Vec3 operator - (const Vec3& lhs, const Vec3& rhs) {
    return lhs + (-rhs);
}

inline Vec3 operator * (const Vec3& lhs, const Vec3& rhs) {
    return Vec3(lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2]);
}

inline Vec3 operator * (const Vec3& v, double t) {
    return Vec3(t * v[0], t * v[1], t * v[2]);
}

inline Vec3 operator * (double t, const Vec3& v) {
    return v * t;
}

inline Vec3 operator / (const Vec3& v, double t) {
    return (1.0 / t) * v;
}

inline double dot(const Vec3& u, const Vec3& v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u[1] * v[2] - u[2] * v[1],
                u[2] * v[0] - u[0] * v[2],
                u[0] * v[1] - u[1] * v[0]);
}

inline Vec3 unit_vector(const Vec3& v) {
    return v / v.length();
}

inline Vec3 lerp_vector(double t, const Vec3& start, const Vec3& end) {
    return (1.0 - t) * start + t * end;
}

std::string to_debug(const Vec3& v) {
    std::stringstream ss;
    ss << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
    return ss.str();
}

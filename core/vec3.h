#pragma once

#include <math.h>
#include <stdlib.h>
#include <iostream>

struct vec3 {
    double e[3];

    __host__ __device__ vec3() : e{ 0,0,0 } {}
    __host__ __device__ vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

    __host__ __device__ inline double x() const { return e[0]; }
    __host__ __device__ inline double y() const { return e[1]; }
    __host__ __device__ inline double z() const { return e[2]; }

    __host__ __device__ inline double r() const { return e[0]; }
    __host__ __device__ inline double g() const { return e[1]; }
    __host__ __device__ inline double b() const { return e[2]; }

    __host__ __device__ inline double operator[](int i) const { return e[i]; }
    __host__ __device__ inline double& operator[](int i) { return e[i]; };

    __host__ __device__ inline const vec3& operator+() const { return *this; }
    __host__ __device__ inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }

    __host__ __device__ 
    inline vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    __host__ __device__ 
    inline vec3& operator*=(double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    __host__ __device__ inline
    vec3& operator/=(double t) {
        return *this *= 1 / t;
    }

    __host__ __device__
    double length() const {
        return std::sqrt(lengthSquared());
    }

    __host__ __device__
    double lengthSquared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    /// <returns>True if the vector is close to zero in all dimensions</returns>
    bool nearZero() const {
        auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }

    static vec3 random() {
        return vec3(randomDouble(), randomDouble(), randomDouble());
    }

    __host__ __device__
    static vec3 random(double min, double max) {
        return vec3(randomDouble(min, max), randomDouble(min, max), randomDouble(min, max));
    }
};


#pragma region Vector Utility Functions

inline std::istream& operator>>(std::istream& is, vec3& t) {
    is >> t.e[0] >> t.e[1] >> t.e[2];
    return is;
}

inline std::ostream& operator<<(std::ostream& os, const vec3& t) {
    os << t.e[0] << " " << t.e[1] << " " << t.e[2];
    return os;
}

__host__ __device__
inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

__host__ __device__
inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

__host__ __device__
inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

__host__ __device__
inline vec3 operator*(double t, const vec3& v) {
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

__host__ __device__
inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

__host__ __device__
inline vec3 operator/(const vec3& v, double t) {
    return (1 / t) * v;
}

__host__ __device__
inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

__host__ __device__
inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

__host__ __device__
inline vec3 unitVector(const vec3& v) {
    return v / v.length();
}

__host__ __device__
inline vec3 randomUnitVector() {
    while (true) {
        auto p = vec3::random(-1, 1);
        auto lensq = p.lengthSquared();
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

__host__ __device__
inline vec3 randomOnHemisphere(const vec3& normal) {
    vec3 on_unit_sphere = randomUnitVector();
    if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

#pragma endregion


// Alias. For clarity in reading code
using point3 = vec3;

// CUDA Raytracer
// Jupiter Sinclair Chong

#pragma once

#include "vec3.h"

struct ray {
    __device__ ray() {}
    __device__ ray(const point3& origin, const vec3& direction) : _origin(origin), dir(direction) {}

    __device__ const point3& origin() const { return _origin; }
    __device__ const vec3& direction() const { return dir; }

    __device__ point3 at(double t) const { return _origin + t * dir; }

private:
    point3 _origin;
    vec3 dir;
};
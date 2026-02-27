
#pragma once

// C++ Standard
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

// C++ Std Usings
using std::make_shared;
using std::shared_ptr;

// Constants
__device__ const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double deg2Rad(double degrees) {
    return degrees * pi / 180.0;
}

inline double randomDouble() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double randomDouble(double min, double max) {
    return min + (max - min) * randomDouble();
}

// Common Headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

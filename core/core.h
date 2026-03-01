// CUDA Raytracer
// Jupiter Sinclair Chong

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

// CUDA
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <curand_kernel.h>

#define checkCudaErrors(val) Core::check_cuda( (val), #val, __FILE__, __LINE__ )

// Constants
__device__ const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

__host__ __device__
inline double deg2Rad(double degrees) {
    return degrees * pi / 180.0;
}

__host__ __device__
inline double randomDouble() {
    return std::rand() / (RAND_MAX + 1.0);
}

__host__ __device__
inline double randomDouble(double min, double max) {
    return min + (max - min) * randomDouble();
}

// Common Headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#include "camera.h"
#include "hitable.h"
#include "sphere.h"

// Keyword : __device__ : only used on GPU
// Keyword : threadIdx  : Contains the index of the current thread within the block
// Keyword : blockIdx   : Contains the index of the current block within the grid
// Keyword : blockDim   : Contains the number of threads in the block
// hmm I'm starting to see a pattern...

// Kernals!
namespace Core {

    // This is lowkey bad actually lol. I need to see where things go out of bounds
    // Checking the CUDA error codes whenever we call a CUDA function
    // https://developer.nvidia.com/blog/accelerated-ray-tracing-cuda/
    void check_cuda(cudaError_t result, char const* const func, const char* const file, int const line) {
        if (result) {
            std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
                file << ":" << line << " '" << func << "' \n";
            // Make sure we call CUDA Device Reset before exiting
            cudaDeviceReset();
            exit(99);
        }
    }

    // Allocate scene objects on GPU
    __global__ void createWorld(Hitable** list, Hitable** world) {
        // This ensures it only ever gets made once
        if (threadIdx.x == 0 && blockIdx.x == 0) {
            // TODO Why do we do this 

            //list->add(new Sphere(vec3(0, 0, -1), 0.5));
            //list->add(make_shared<Sphere>(vec3(0, -100.5, -1), 100));

            *(list) = new Sphere(vec3(0, 0, -1), 0.5);
            *(list + 1) = new Sphere(vec3(0, -100.5, -1), 100);

            *world = new HitableList(list, 2);
        }
    }

    // Deallocate scene
    __global__ void freeWorld(Hitable** list, Hitable** world) {
        delete* (list);
        delete* (list + 1);
        delete world;
    }



}

// CUDA Raytracer
// Jupiter Sinclair Chong

// C++
#include <iostream>
#include <time.h>
#include <stdio.h>

// CUDA
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

// Core Utility
#include "core/camera.h"
#include "core/core.h"
#include "core/hitable.h"
#include "core/sphere.h"

typedef Hitable* HitableObject;
//typedef shared_ptr<Hitable> HitableObject;
typedef std::vector<HitableObject> HitableObjects;

// Checking the CUDA error codes whenever we call a CUDA function
// https://developer.nvidia.com/blog/accelerated-ray-tracing-cuda/
#define checkCudaErrors(val) check_cuda( (val), #val, __FILE__, __LINE__ )
void check_cuda(cudaError_t result, char const* const func, const char* const file, int const line) {
    if (result) {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
            file << ":" << line << " '" << func << "' \n";
        // Make sure we call CUDA Device Reset before exiting
        cudaDeviceReset();
        exit(99);
    }
}

// Keyword : __device__ : only used on GPU
// Keyword : threadIdx  : Contains the index of the current thread within the block
// Keyword : blockIdx   : Contains the index of the current block within the grid
// Keyword : blockDim   : Contains the number of threads in the block
// hmm I'm starting to see a pattern...

__global__ void freeWorld(Hitable** list, Hitable** world) {
    delete* (list);
    delete* (list + 1);
    delete world;
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


__device__ color getColor(const ray& r, Hitable** world) {
    HitInfo hitInfo;
    if ((*world)->hit(r, interval(0.001, infinity), hitInfo)) {
        return 0.5f * vec3(hitInfo.normal.x() + 1.0f, hitInfo.normal.y() + 1.0f, hitInfo.normal.z() + 1.0f);
    }
    else {
        float t = 0.5f * (unitVector(r.direction()).y() + 1.0f);
        return (1.0f - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }

    //vec3 unit_direction = unitVector(r.direction());
    //float t = 0.5f * (unit_direction.y() + 1.0f);
    //return (1.0f - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

// Kernal to render to the framebuffer
__global__ void render(color* frameBuffer, int max_x, int max_y, vec3 lowerLeftCorner, vec3 horizontal, vec3 vertical, vec3 origin, Hitable** world) {
    // Image coords
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    // Out of buffer bounds
    if ((i >= max_x) || (j >= max_y)) return;

    int pixel_index = j * max_x * 3 + i * 3;
    float u = float(i) / float(max_x);
    float v = float(j) / float(max_y);

    // Assigning Colours, wow it's a shader...
    ray r(origin, lowerLeftCorner + u * horizontal + v * vertical);
    frameBuffer[pixel_index] = getColor(r, world);
}

int main()
{
    // Image size
    int width = 256, height = 256;
    
    // These can be changed. Threads per block
    int threadX = 8, threadY = 8;

    std::cerr << "[Image Size] \t X : " << width << " \tY : " << height << std::endl;
    std::cerr << "[Blocks] \t X " << threadX << " \tY : " << threadY << std::endl;

    // Calculate the size of the FrameBuffer in bytes
    int num_pixels = width * height;
    size_t frameBuffer_size = 3 * num_pixels * sizeof(color);

    // Allocate FrameBuffer on GPU using Unified Memory (can be accessed by GPU & CPU)
    color* frameBuffer;
    checkCudaErrors(cudaMallocManaged((void**)&frameBuffer, frameBuffer_size));

    // Allocate world data on GPU
    // TODO Convert List to C cringe
    Hitable** device_HitableList;   // device_ prefix is device-only data
    checkCudaErrors(cudaMalloc((void**)&device_HitableList, 2 * sizeof(Hitable*)));

    Hitable** device_World;
    checkCudaErrors(cudaMalloc((void**)&device_World, sizeof(Hitable*)));

    createWorld<<<1,1>>>(device_HitableList, device_World);

    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    /*Start Timer*/
    clock_t start, stop;
    start = clock();

    dim3 blocks(width / threadX + 1, height / threadY + 1);
    dim3 threads(threadX, threadY);

    // Render to FrameBuffer
    render <<<blocks, threads>>> 
    (
        frameBuffer, width, height, 
        vec3(-2.0, -1.0, -1.0),
        vec3(4.0, 0.0, 0.0),
        vec3(0.0, 2.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        device_World
    );

    checkCudaErrors(cudaGetLastError());

    // Wait for GPU to finish before accessing
    checkCudaErrors(cudaDeviceSynchronize());

    /*End Timer*/
    stop = clock();
    double timer_seconds = ((double)(stop - start)) / CLOCKS_PER_SEC;
    std::cerr << "[Render] " << timer_seconds << " seconds.\n";

    // Output frameBuffer as Image
    std::cout << "P3\n" << width << " " << height << "\n255\n";
    for (int j = height - 1; j >= 0; j--) {
        for (int i = 0; i < width; i++) {
            std::clog << "\rScanlines remaining:\t " << (j) << ' ' << std::flush;

            size_t pixel_index = j * width + i;
            int ir = int(255.99 * frameBuffer[pixel_index].r());
            int ig = int(255.99 * frameBuffer[pixel_index].g());
            int ib = int(255.99 * frameBuffer[pixel_index].b());

            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }

    std::clog << "\n[Render Finished]\n";

    // Be free!
    checkCudaErrors(cudaDeviceSynchronize());

    freeWorld <<<1, 1 >>> (device_HitableList, device_World);
    
    checkCudaErrors(cudaGetLastError());

    checkCudaErrors(cudaFree(device_HitableList));
    checkCudaErrors(cudaFree(device_World));
    checkCudaErrors(cudaFree(frameBuffer));

    cudaDeviceReset();

    return 0;
}
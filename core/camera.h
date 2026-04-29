// CUDA Raytracer
// Jupiter Sinclair Chong

#pragma once

#include "hitable.h"
#include "framebuffer.h"
#include "core.h"

#include <time.h>
#include <sstream>

#define checkCudaErrors(val) Core::check_cuda( (val), #val, __FILE__, __LINE__ )

// Kernels used by Camera
namespace Core {
    __global__ void renderInit(int maxX, int maxY, curandState* randState) {
        // Get image coords, do OOB check, and pixel index in array
        int i = threadIdx.x + blockIdx.x * blockDim.x;
        int j = threadIdx.y + blockIdx.y * blockDim.y;
        if ((i >= maxX) || (j >= maxY)) return;

        int pixelIndex = j * maxX + i;

        // Each thread gets same seed, a different sequence number, no offset
        curand_init(1984, pixelIndex, 0, &randState[pixelIndex]);
    }

    __device__ color getColor(const ray& r, Hitable** world) {
        //vec3 unitDirection = unitVector(r.direction());
        //float t = 0.5f * (unitDirection.y() + 1.0f);
        //return (1.0f - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);

        HitInfo hitInfo;
        //if ((*world)->hit(r, interval(0.001, infinity), hitInfo)) {
        if ((*world)->hit(r, 0.0, FLT_MAX, hitInfo)) {
            return 0.5f * vec3(hitInfo.normal.x() + 1.0f, hitInfo.normal.y() + 1.0f, hitInfo.normal.z() + 1.0f);
        }
        else {
            float t = 0.5f * (unitVector(r.direction()).y() + 1.0f);
            return (1.0f - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
        }

    }

    // Render to the framebuffer
    __global__ void render(color* frameBuffer, int max_x, int max_y, vec3 lowerLeftCorner, vec3 horizontal, vec3 vertical, vec3 origin, Hitable** world/*, curandState* randState*/) {
        // Image coords
        int i = threadIdx.x + blockIdx.x * blockDim.x;
        int j = threadIdx.y + blockIdx.y * blockDim.y;

        // Out of buffer bounds
        if ((i >= max_x) || (j >= max_y)) return;

        int pixel_index = j * max_x + i;
        float u = float(i) / float(max_x);
        float v = float(j) / float(max_y);

        // Assigning Colours, wow it's a shader...
        ray r(origin, lowerLeftCorner + u * horizontal + v * vertical);
        frameBuffer[pixel_index] = Core::getColor(r, world);
    }

}

struct Camera {
    Framebuffer* frameBuffer;
    double aspectRatio = 1.0;
    int imageWidth = 100;
    int imageHeight = 100;
    int samplesPerPixel = 10;
    int maxDepth = 10;

    // Threads per block. Kinda arbitrary
    int threadX = 8, threadY = 8;

    Camera(){}

    // TODO Should I have CUDA code in here?
    void render(Hitable** world) {
        /*Start Timer*/
        clock_t start, stop;
        start = clock();
        
        initialize();

        std::cerr << printInfo().c_str();

        Core::render<<<blocks, threads>>>
        (
            frameBuffer->buffer, imageWidth, imageHeight,
            vec3(-2.0, -1.0, -1.0),
            vec3(4.0, 0.0, 0.0),
            vec3(0.0, 2.0, 0.0),
            vec3(0.0, 0.0, 0.0),
            world
        );

        cudaGetLastError();
        cudaDeviceSynchronize();

        /*End Timer*/
        stop = clock();
        double timer_seconds = ((double)(stop - start)) / CLOCKS_PER_SEC;
        std::cerr << "[Render Finished] " << std::endl << "\tTime\t: "  << timer_seconds << " seconds\n";
    }

    void printPPM() {
        frameBuffer->printPPM();
    }

    std::string printInfo() {
        std::ostringstream output;

        output << "[Camera Info]\n";
        output << "\tImage Size: X : " << imageWidth    << " \tY : " << imageHeight << std::endl;
        output << "\tBlocks    : X : " << threadX       << " \tY : " << threadY     << std::endl;

        return output.str();
    }

private:
    double pixelSampleScale;
    vec3 pixelDeltaU, pixelDeltaV;
    point3 cameraCenter, topLeftPixelPos;
    dim3 blocks, threads;

    // Update member vars before rendering
    void initialize() {
        // Member vars
        imageHeight = int(imageWidth / aspectRatio);
        imageHeight = (imageHeight < 1) ? 1 : imageHeight;
        pixelSampleScale = 1.0 / samplesPerPixel;
        cameraCenter = point3(0, 0, 0);

        dim3 blocks(imageWidth / threadX + 1, imageHeight / threadY + 1);
        dim3 threads(threadX, threadY);

        frameBuffer = new Framebuffer(imageWidth, imageHeight);

        // 
        double focalLength = 1.;
        double viewportHeight = 2.0;
        double viewportWidth = viewportHeight * (double(imageWidth) / imageHeight);

        // Calculate vectors across (L -> R) the horizontal and (Up -> Down) the vertical viewport edges
        vec3 viewportU = vec3(viewportWidth, 0, 0);
        vec3 viewportV = vec3(0, -viewportHeight, 0);

        // Calculate delta vectors from pixel to pixel
        pixelDeltaU = viewportU / imageWidth;
        pixelDeltaV = viewportV / imageHeight;

        // Top left pixel
        vec3 viewportTopLeft = cameraCenter - vec3(0, 0, focalLength) - viewportU / 2 - viewportV / 2;
        topLeftPixelPos = viewportTopLeft + 0.5 * (pixelDeltaU + pixelDeltaV);
    }

    // Construct a camera ray originating from the origin and directed at randomly sampled point around the pixel location i, j.
    __device__ ray getRay(int i, int j) const {

        auto offset = sampleSquare();
        auto pixel_sample = topLeftPixelPos
            + ((i + offset.x()) * pixelDeltaU)
            + ((j + offset.y()) * pixelDeltaV);

        auto ray_origin = cameraCenter;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    __device__ vec3 sampleSquare() const {
        return vec3(randomDouble() - 0.5, randomDouble() - 0.5, 0);
    }

    // Per Pixel... wow it's a fragment shader
    __device__ color rayColor(const ray& r, int depth, const Hitable& world) const {
        // Bounce limit 
        if (depth <= 0) return color(0, 0, 0);

        HitInfo hitInfo;

        if (world.hit(r, interval(0.001, infinity), hitInfo)) {
            vec3 direction = hitInfo.normal + randomUnitVector();
            return 0.1 * rayColor(ray(hitInfo.p, direction), depth - 1, world);
            //return shadeNormal(hitInfo.normal);
        }

        vec3 unitDirection = unitVector(r.direction());
        double a = 0.5 * (unitDirection.y() + 1.0);

        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

//namespace Core {
//
//
//// Render to the framebuffer
//__global__ void render(Camera* cam, vec3 origin, Hitable** world/*, curandState* randState*/) {
//    // Image coords
//    int i = threadIdx.x + blockIdx.x * blockDim.x;
//    int j = threadIdx.y + blockIdx.y * blockDim.y;
//
//    int maxX = cam->imageWidth;
//    int maxY = cam->imageHeight;
//
//    // Out of buffer bounds
//    if ((i >= maxX) || (j >= maxY)) return;
//
//    int pixel_index = j * maxY + i;
//    float u = float(i) / float(maxY);
//    float v = float(j) / float(maxY);
//
//    // Assigning Colours, wow it's a shader...
//    //ray r(origin, lowerLeftCorner + u * horizontal + v * vertical);
//    //frameBuffer[pixel_index] = Core::getColor(r, world);
//}
//
//__global__ void testKernel() {
//}
//
//}
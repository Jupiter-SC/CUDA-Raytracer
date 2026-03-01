// CUDA Raytracer
// Jupiter Sinclair Chong

// C++
#include <iostream>
#include <time.h>
#include <stdio.h>

// Core Utility
#include "core/core.h"

typedef Hitable* HitableObject;
//typedef shared_ptr<Hitable> HitableObject;
typedef std::vector<HitableObject> HitableObjects;


// Using "device_" prefix for device-only data

int main()
{
    Camera cam;
    cam.imageWidth = 400; 

    // Allocate random state
    //curandState* device_randState;
    //checkCudaErrors(cudaMalloc((void**)&device_randState, cam.imageWidth * cam.imageHeight * sizeof(curandState)));

    // Allocate world data on GPU
    Hitable** device_HitableList;
    checkCudaErrors(cudaMalloc((void**)&device_HitableList, 2 * sizeof(Hitable*)));

    Hitable** device_World;
    checkCudaErrors(cudaMalloc((void**)&device_World, sizeof(Hitable*)));

    Core::createWorld<<<1,1>>>(device_HitableList, device_World);

    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    // Initting random
    //renderInit<<<blocks, threads>>>(width, heigh, device_randState);

    //checkCudaErrors(cudaGetLastError());
    //checkCudaErrors(cudaDeviceSynchronize());

    // Render to FrameBuffer
    
    cam.render(device_World);

    dim3 blocks(cam.imageWidth / cam.threadX + 1, cam.imageHeight / cam.threadY + 1);
    dim3 threads(cam.threadX, cam.threadY);
    Core::render <<<blocks, threads>>>
    (
        cam.frameBuffer.buffer, cam.imageWidth, cam.imageHeight, 
        vec3(-2.0, -1.0, -1.0),
        vec3(4.0, 0.0, 0.0),
        vec3(0.0, 2.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        device_World
    );
    

    // Wait for GPU to finish before accessing
    cudaGetLastError();
    cudaDeviceSynchronize();

    // Output frameBuffer as Image
    std::cout << "P3\n" << cam.imageWidth<< " " << cam.imageHeight << "\n255\n";
    for (int j = cam.imageHeight - 1; j >= 0; j--) {
        for (int i = 0; i < cam.imageWidth; i++) {
            std::clog << "\rScanlines remaining:\t " << (j) << ' ' << std::flush;

            size_t pixel_index = j * cam.imageWidth + i;

            //int ir = frameBuffer[0].r();
            //int ig = 0;
            //int ib = 0;

            int ir = int(255.99 * cam.frameBuffer.buffer[pixel_index].r());
            int ig = int(255.99 * cam.frameBuffer.buffer[pixel_index].g());
            int ib = int(255.99 * cam.frameBuffer.buffer[pixel_index].b());

            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }

    cam.printPPM();


    std::clog << "\n[Render Finished]\n";

    // Be free!
    checkCudaErrors(cudaDeviceSynchronize());

    Core::freeWorld<<<1, 1 >>>(device_HitableList, device_World);
    
    checkCudaErrors(cudaGetLastError());

    cudaFree(device_World);
    cudaFree(device_HitableList);

    cudaDeviceReset();

    return 0;
}
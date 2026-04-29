// CUDA Raytracer
// Jupiter Sinclair Chong

#include "color.h"
#include "core.h"

#define checkCudaErrors(val) Core::check_cuda( (val), #val, __FILE__, __LINE__ )

struct Framebuffer {
	int width = 100, height = 100;
	color* buffer;

	Framebuffer() {}

	// Allocate FrameBuffer on GPU using Unified Memory (can be accessed by GPU & CPU)
	// Calculate the size of the FrameBuffer in bytes
	__host__
	Framebuffer(int _width, int _height) {
		width = _width;
		height = _height;

		int numPixels = width * height;
		size_t frameBuffer_size = numPixels * sizeof(color);

		cudaMallocManaged((void**)&buffer, frameBuffer_size);
	}

	__host__
	~Framebuffer() {
		cudaFree(buffer);
	}

	// TODO Add accessors
	__host__ __device__ inline color operator[](int i) const { return buffer[i]; }

	// Cout framebuffer. So it can be output as image
	void printPPM() {
		std::cout << "P3\n" << width << " " << height << "\n255\n";
		for (int j = height - 1; j >= 0; j--) {
		    for (int i = 0; i < width; i++) {
		        std::clog << "\rScanlines remaining:\t " << (j) << ' ' << std::flush;

		        size_t pixel_index = j * width + i;

		        int ir = int(255.99 * buffer[pixel_index].r());
		        int ig = int(255.99 * buffer[pixel_index].g());
		        int ib = int(255.99 * buffer[pixel_index].b());

		        std::cout << ir << " " << ig << " " << ib << "\n";
		    }
		}
	}
};
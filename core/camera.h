// Jupiter Sinclair Chong

#pragma once

#include "hitable.h"
#include <time.h>

struct Camera {
    double aspectRatio = 1.0;
    int imageWidth = 100;
    int samplesPerPixel = 10;
    int maxDepth = 10;

    // TODO move CUDA code into here
    void render(const Hitable& world) {
        initialize();

        std::cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

        for (int j = 0; j < imageHeight; j++) {
            std::clog << "\rScanlines remaining:\t " << (imageHeight - j) << ' ' << std::flush;

            for (int i = 0; i < imageWidth; i++) {
                // Anti Aliasing
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samplesPerPixel; sample++) {
                    //ray r = getRay(i, j);
                    //pixel_color += rayColor(r, maxDepth, world);
                }
                writeColor(std::cout, pixelSampleScale * pixel_color);
            }
        }

        std::clog << "\r[Render Finished]\n";
    }

private:
    int    imageHeight = 100;
    double pixelSampleScale;
    vec3 pixelDeltaU, pixelDeltaV;
    point3 cameraCenter, topLeftPixelPos;

    void initialize() {
        // Member vars
        imageHeight = int(imageWidth / aspectRatio);
        imageHeight = (imageHeight < 1) ? 1 : imageHeight;
        pixelSampleScale = 1.0 / samplesPerPixel;
        cameraCenter = point3(0, 0, 0);

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
        vec3 viewport_upper_left = cameraCenter - vec3(0, 0, focalLength) - viewportU / 2 - viewportV / 2;
        topLeftPixelPos = viewport_upper_left + 0.5 * (pixelDeltaU + pixelDeltaV);
    }

    // Construct a camera ray originating from the origin and directed at randomly sampled point around the pixel location i, j.
    //ray getRay(int i, int j) const {

    //    auto offset = sampleSquare();
    //    auto pixel_sample = topLeftPixelPos
    //        + ((i + offset.x()) * pixelDeltaU)
    //        + ((j + offset.y()) * pixelDeltaV);

    //    auto ray_origin = cameraCenter;
    //    auto ray_direction = pixel_sample - ray_origin;

    //    return ray(ray_origin, ray_direction);
    //}

    //// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    //vec3 sampleSquare() const {
    //    return vec3(randomDouble() - 0.5, randomDouble() - 0.5, 0);
    //}

    //// Per Pixel... wow it's a fragment shader
    //color rayColor(const ray& r, int depth, const Hitable& world) const {
    //    // Bounce limit 
    //    if (depth <= 0) return color(0, 0, 0);

    //    HitInfo hitInfo;

    //    if (world.hit(r, interval(0.001, infinity), hitInfo)) {
    //        vec3 direction = hitInfo.normal + randomUnitVector();
    //        return 0.1 * rayColor(ray(hitInfo.p, direction), depth - 1, world);
    //        //return shadeNormal(hitInfo.normal);
    //    }

    //    vec3 unitDirection = unitVector(r.direction());
    //    double a = 0.5 * (unitDirection.y() + 1.0);

    //    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    //}
};

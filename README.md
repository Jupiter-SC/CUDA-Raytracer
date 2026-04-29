# CUDA_Raytracer

A software rasterizer using NVidia CUDA.
Currently it shades two spheres by their normals.

Compared with [CPU implementation](https://github.com/Jupiter-SC/Raytracing-Weekend)
* 400x225 image. 2 Spheres shading their normals. No multisampling
* Time doesn’t include writing to the PPM file, it’s just going to the framebuffers

| Device  | Speed (in seconds) |
| ------------- | ------------- |
| CPU (12th Gen Intel i5-12450)  | ~ .062  |
| GPU (Nvidia RTX 4050) - 1 thread  | ~ .015   |
| GPU - 8 threads  | .001   |
| GPU - 16 threads  | 0 or .001 (lack of precision) |

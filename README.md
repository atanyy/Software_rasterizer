## Description

A software rasterizer made using the C programming language. The only libraries used are the win32 libraries for window creation and window management, C standard libraries for file I/O and math functions.

### Note: All image file inputs and output in the BMP (Windows Bitmap) format.

#### Some examples created using the rasterizer 

A textured bunny:

![Bunny Textured](/data/bunny.png)

A textured buddha:

![Buddha Textured](/data/buddha.png)

A textured cube:

![Textured Cube](/data/textured_cube.gif)

A flat shaded bunny:

![Flat-shaded bunny](/data/bunny_flat_shaded.png)

A flat shaded cube:

![Flat-shaded cube](/data/cube_flat_shaded.gif)

## Feature List

* Line drawing using the Bresenham line drawing and mid-point line drawing algorithms.
* Perspective texture mapping using pre-computed gradients.
* Triangle fill using the flat-bottom, flat-top method.
* Memory Arena for storing program persistant data.
* Quick sort using the Median-of-three method to sort for Painter's algorithm.
* Flat Shading.

## Currently Working On

* Implementing clipping
* Z-buffer
* Camera Movement
* SIMD for parallelization
* Different shading techniques: Gouraud, Phong?


# minimal-rasterizer (Experimental)

This project is a **small, experimental software renderer** inspired by  
Dmitry V. Sokolov’s *Tiny Renderer* tutorial:  
https://haqr.eu/

The goal of this project was not to build a production-ready renderer, but to
explore the mathematics and core ideas behind rendering pipelines
(transformations, projections, rasterization, and basic lighting).

As such, the renderer is intentionally minimal, incomplete, and intended primarily
as a learning exercise.

---
## How to Compile

Example (using `g++`):

```bash
g++ main.cpp lib/renderer.cpp lib/extraMath.cpp lib/objLoader.cpp -o rasterizer
```
You may need to adjust paths or files depending on your setup.

---
## Project Scope

- Implemented from scratch (no GPU APIs)
- Focus on:
  - Model/View/Projection transformations
  - Rasterization
  - Z-buffering
  - Flat shading
  - Basic diffuse + specular lighting
- Not intended for:
  - Real-time use
  - Production rendering
  - Feature completeness

Some parts may be fragile or unfinished by design.

---

## Quick Usage

### 1. Create a renderer instance

`lib/renderer.h` exposes a `Render` class.

```cpp
Render r(width, height, x, y);
// x and y denote the horizontal and vertical offset
// from the top-left corner of the screen.
```

### 2. Load an OBJ file
```cpp
r.loadOBJ("file.obj");
```

### 3. Define camera parameters
```cpp
Vec3 center = {0, 0, 0};
Vec3 up     = {0, -1, 0};
Vec3 eye    = {4, 0, 4};
```

### 4. Set up the Camera
```cpp
r.bringCamera(eye, up, center);
```

### 5. Render a frame
```cpp
TGAImage frame = r.renderFrame();        // basic rendering
// or
TGAImage frame = r.phongReflection(eye); // diffuse + specular lighting
```

### 6. Save the output
```cpp
frame.write_tga_file("output.tga");
```
---


## Notes on Camera Implementation
The `ModelView` transformation is built using the object center, not the eye
position directly.
This means the camera behavior is not a direct equivalent of `gluLookAt`.

As a result, zooming out does not scale the rendered object as expected.

The camera model should be considered an experimental variation.


---

## Disclaimer
This project is best viewed as a learning and experimentation sandbox.
It is not actively developed further and is not intended to be used as a library
or framework.

If you’re interested in software rendering, graphics math, or building a renderer
from first principles, you may still find parts of this code useful.

---

## Third-Party Code

This project uses `tgaimage.h` + `tgaimage.cpp` from Dmitry V. Sokolov’s *Tiny Renderer* project
for basic image output functionality.

`tgaimage.h` is not original work and is included solely for educational purposes.
All credit for this component goes to its original author.

---

## Acknowledgements
- Dmitry V. Sokolov — *Tiny Renderer*


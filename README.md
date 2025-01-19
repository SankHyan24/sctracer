# sctracer
Chuan's Path Tracer

Requirements:
- CMake
- C++17 compiler
- Python 3.6+
- OpenGL GPU


## Class Structure

Window
- Scene
  - Camera
  - Object
    - Sphere
    - Plane
    - Triangle
    - Mesh
  - Light
    - PointLight
    - AreaLight
  - Material
    - Lambertian
    - Metal
    - Dielectric
    - DiffuseLight
  - BVH
  - Texture
    - ImageTexture
    - CheckerTexture
    - NoiseTexture
  - Renderer
    - PathTracer

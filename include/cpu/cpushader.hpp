#pragma once
#include <glm/glm.hpp>

namespace scTracer::CPU
{

#define PI 3.14159265358979323
#define INV_PI 0.31830988618379067
#define TWO_PI 6.28318530717958648
#define INV_TWO_PI 0.15915494309189533
#define INV_4_PI 0.07957747154594766
#define EPS 0.0003
#define INF 1000000.0

#define QUAD_LIGHT 0.f
#define SPHERE_LIGHT 1.f
#define DISTANT_LIGHT 2.f

#define ALPHA_MODE_OPAQUE 0
#define ALPHA_MODE_BLEND 1
#define ALPHA_MODE_MASK 2

#define MEDIUM_NONE 0
#define MEDIUM_ABSORB 1
#define MEDIUM_SCATTER 2
#define MEDIUM_EMISSIVE 3

    extern glm::uvec4 seed;
    extern glm::ivec2 pixel;

    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;
        Ray() : origin(0.0f), direction(0.0f) {}
        Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {}
    };

    struct Medium
    {
        int type;
        float density;
        glm::vec3 color;
        float anisotropy;
    };

    struct Material
    {
        glm::vec3 baseColor;
        float opacity;
        int alphaMode;
        float alphaCutoff;
        glm::vec3 emission;
        float anisotropic;
        float metallic;
        float roughness;
        float subsurface;
        float specularTint;
        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatRoughness;
        float specTrans;
        float ior;
        float ax;
        float ay;
        Medium medium;
    };

    struct Camera
    {
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 forward;
        glm::vec3 position;
        float fov;
        float focalDist;
        float aperture;
    };

    struct Light
    {
        glm::vec3 position;
        glm::vec3 emission;
        glm::vec3 u;
        glm::vec3 v;
        float radius;
        float area;
        float type;
    };

    struct State
    {
        int depth;
        float eta;
        float hitDist;

        glm::vec3 fhp;
        glm::vec3 normal;
        glm::vec3 ffnormal;
        glm::vec3 tangent;
        glm::vec3 bitangent;

        bool isEmitter;

        glm::vec2 texCoord;
        int matID;
        Material mat;
        Medium medium;
    };

    struct ScatterSampleRec
    {
        glm::vec3 L;
        glm::vec3 f;
        float pdf;
    };

    struct LightSampleRec
    {
        glm::vec3 normal;
        glm::vec3 emission;
        glm::vec3 direction;
        float dist;
        float pdf;
        LightSampleRec() : normal(0.0f), emission(0.0f), direction(0.0f), dist(0.0f), pdf(0.0f) {}
    };

    // RNG from code by Moroz Mykhailo (https://www.shadertoy.com/view/wltcRS)

    // internal RNG state

    void InitRNG(glm::vec2 p, int frame);

    void pcg4d(glm::uvec4 &v);

    float rand();

    glm::vec3 FaceForward(glm::vec3 a, glm::vec3 b);

    float Luminance(glm::vec3 c);
}

namespace scTracer::CPU
{
    // bool ClosestHit(Ray r, State &state, LightSampleRec lightSample, glm::vec3 &debugger);
}
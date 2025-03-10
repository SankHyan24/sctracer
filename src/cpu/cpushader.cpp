#include <cpu/cpushader.hpp>
namespace scTracer::CPU
{
    glm::uvec4 seed;
    glm::ivec2 pixel;

        void InitRNG(glm::vec2 p, int frame)
    {
        pixel = glm::ivec2(p);
        seed = glm::uvec4(p.x, p.y, uint32_t(frame), uint32_t(p.x) + uint32_t(p.y));
    }

    void pcg4d(glm::uvec4 &v)
    {
        v = v * 1664525u + 1013904223u;
        v.x += v.y * v.w;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        v.w += v.y * v.z;
        v = v ^ (v >> 16u);
        v.x += v.y * v.w;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        v.w += v.y * v.z;
    }

    float rand()
    {
        pcg4d(seed);
        return float(seed.x) / float(0xffffffffu);
    }

    glm::vec3 FaceForward(glm::vec3 a, glm::vec3 b)
    {
        return dot(a, b) < 0.0 ? -b : b;
    }

    float Luminance(glm::vec3 c)
    {
        return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
    }
}
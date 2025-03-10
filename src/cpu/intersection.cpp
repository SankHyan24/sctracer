#include <cpu/integrator.hpp>

namespace scTracer::CPU
{
    float Integrator::AABBIntersect(glm::vec3 minCorner, glm::vec3 maxCorner, Ray r)
    {
        glm::vec3 invDir = {
            1.0f / r.direction.x,
            1.0f / r.direction.y,
            1.0f / r.direction.z};

        glm::vec3 f = (maxCorner - r.origin) * invDir;
        glm::vec3 n = (minCorner - r.origin) * invDir;

        glm::vec3 tmax = glm::max(f, n);
        glm::vec3 tmin = glm::min(f, n);

        float t1 = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
        float t0 = glm::max(tmin.x, glm::max(tmin.y, tmin.z));

        return (t1 >= t0) ? (t0 > 0.f ? t0 : t1) : -1.0;
    }
}
#include <cpu/integrator.hpp>

namespace scTracer::CPU
{
    float Integrator::SphereIntersect(float rad, glm::vec3 pos, Ray r)
    {
        glm::vec3 op = pos - r.origin;
        float eps = 0.001;
        float b = glm::dot(op, r.direction);
        float det = b * b - glm::dot(op, op) + rad * rad;
        if (det < 0.0)
            return INF;

        det = sqrt(det);
        float t1 = b - det;
        if (t1 > eps)
            return t1;

        float t2 = b + det;
        if (t2 > eps)
            return t2;

        return INF;
    }
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

    float Integrator::RectIntersect(glm::vec3 pos, glm::vec3 u, glm::vec3 v, glm::vec4 plane, Ray r)
    {
        glm::vec3 n = glm::vec3(plane);
        float dt = glm::dot(r.direction, n);
        float t = (plane.w - dot(n, r.origin)) / dt;

        if (t > EPS)
        {
            glm::vec3 p = r.origin + r.direction * t;
            glm::vec3 vi = p - pos;
            float a1 = glm::dot(u, vi);
            if (a1 >= 0.0 && a1 <= 1.0)
            {
                float a2 = glm::dot(v, vi);
                if (a2 >= 0.0 && a2 <= 1.0)
                    return t;
            }
        }

        return INF;
    }
}
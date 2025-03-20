#include <cpu/integrator.hpp>

namespace scTracer::CPU
{
    glm::vec3 Integrator::UniformSampleHemisphere(float r1, float r2)
    {
        float r = sqrt(glm::max(0.0, 1.0 - r1 * r1));
        float phi = TWO_PI * r2;
        return glm::vec3(r * cos(phi), r * sin(phi), r1);
    }

    float Integrator::PowerHeuristic(float a, float b)
    {
        float t = a * a;
        return t / (b * b + t);
    }

    void Integrator::Onb(glm::vec3 N, glm::vec3 &T, glm::vec3 B)
    {
        glm::vec3 up = glm::abs(N.z) < 0.9999999 ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        T = glm::normalize(cross(up, N));
        B = glm::cross(N, T);
    }

    void Integrator::SampleSphereLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample)
    {
        float r1 = rand();
        float r2 = rand();

        glm::vec3 sphereCentertoSurface = scatterPos - light.position;
        float distToSphereCenter = glm::length(sphereCentertoSurface);
        glm::vec3 sampledDir;

        // TODO: Fix this. Currently assumes the light will be hit only from the outside
        sphereCentertoSurface /= distToSphereCenter;
        sampledDir = UniformSampleHemisphere(r1, r2);
        glm::vec3 T{0.f}, B{0.f};
        Onb(sphereCentertoSurface, T, B);
        sampledDir = T * sampledDir.x + B * sampledDir.y + sphereCentertoSurface * sampledDir.z;

        glm::vec3 lightSurfacePos = light.position + sampledDir * light.radius;

        lightSample.direction = lightSurfacePos - scatterPos;
        lightSample.dist = glm::length(lightSample.direction);
        float distSq = lightSample.dist * lightSample.dist;

        lightSample.direction /= lightSample.dist;
        lightSample.normal = glm::normalize(lightSurfacePos - light.position);
        lightSample.emission = light.emission * float(mScene->lights.size());
        lightSample.pdf = distSq / (light.area * 0.5 * abs(dot(lightSample.normal, lightSample.direction)));
    }

    void Integrator::SampleRectLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample)
    {
        float r1 = rand();
        float r2 = rand();

        glm::vec3 lightSurfacePos = light.position + light.u * r1 + light.v * r2;
        lightSample.direction = lightSurfacePos - scatterPos;
        lightSample.dist = length(lightSample.direction);
        float distSq = lightSample.dist * lightSample.dist;
        lightSample.direction /= lightSample.dist;
        lightSample.normal = normalize(cross(light.u, light.v));
        lightSample.emission = light.emission * float(mScene->lights.size());
        lightSample.pdf = distSq / (light.area * abs(dot(lightSample.normal, lightSample.direction)));
    }

    void Integrator::SampleDistantLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample)
    {
        lightSample.direction = glm::normalize(light.position - glm::vec3(0.0));
        lightSample.normal = glm::normalize(scatterPos - light.position);
        lightSample.emission = light.emission * float(mScene->lights.size());
        lightSample.dist = INF;
        lightSample.pdf = 1.0;
    }

    void Integrator::SampleOneLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample)
    {
        int type = int(light.type);

        if (type == QUAD_LIGHT)
            SampleRectLight(light, scatterPos, lightSample);
        else if (type == SPHERE_LIGHT)
            SampleSphereLight(light, scatterPos, lightSample);
        else
            SampleDistantLight(light, scatterPos, lightSample);
    }

    glm::vec3 Integrator::SampleHG(glm::vec3 V, float g, float r1, float r2)
    {
        float cosTheta;

        if (abs(g) < 0.001)
            cosTheta = 1 - 2 * r2;
        else
        {
            float sqrTerm = (1 - g * g) / (1 + g - 2 * g * r2);
            cosTheta = -(1 + g * g - sqrTerm * sqrTerm) / (2 * g);
        }

        float phi = r1 * TWO_PI;
        float sinTheta = glm::clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        glm::vec3 v1(0.0f), v2(0.0f);
        Onb(V, v1, v2);

        return sinTheta * cosPhi * v1 + sinTheta * sinPhi * v2 + cosTheta * V;
    }

    float Integrator::PhaseHG(float cosTheta, float g)
    {
        float denom = 1 + g * g + 2 * g * cosTheta;
        return INV_4_PI * (1 - g * g) / (denom * sqrt(denom));
    }
    float Integrator::SchlickWeight(float u)
    {
        float m = glm::clamp(1.0 - u, 0.0, 1.0);
        float m2 = m * m;
        return m2 * m2 * m;
    }

    glm::vec3 Integrator::CosineSampleHemisphere(float r1, float r2)
    {
        glm::vec3 dir;
        float r = sqrt(r1);
        float phi = TWO_PI * r2;
        dir.x = r * cos(phi);
        dir.y = r * sin(phi);
        dir.z = sqrt(glm::max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));
        return dir;
    }

    float Integrator::DielectricFresnel(float cosThetaI, float eta)
    {
        float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);

        // Total internal reflection
        if (sinThetaTSq > 1.0)
            return 1.0;

        float cosThetaT = sqrt(glm::max(1.0 - sinThetaTSq, 0.0));

        float rs = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);
        float rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);

        return 0.5f * (rs * rs + rp * rp);
    }

    glm::vec3 Integrator::SampleGGXVNDF(glm::vec3 V, float ax, float ay, float r1, float r2)
    {
        glm::vec3 Vh = glm::normalize(glm::vec3(ax * V.x, ay * V.y, V.z));

        float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
        glm::vec3 T1 = lensq > 0 ? glm::vec3(-Vh.y, Vh.x, 0) * glm::inversesqrt(lensq) : glm::vec3(1, 0, 0);
        glm::vec3 T2 = cross(Vh, T1);

        float r = sqrt(r1);
        float phi = 2.0 * PI * r2;
        float t1 = r * cos(phi);
        float t2 = r * sin(phi);
        float s = 0.5 * (1.0 + Vh.z);
        t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

        glm::vec3 Nh = t1 * T1 + t2 * T2 + sqrtf(glm::max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

        return glm::normalize(glm::vec3(ax * Nh.x, ay * Nh.y, glm::max(0.0f, Nh.z)));
    }

    float Integrator::GTR2Aniso(float NDotH, float HDotX, float HDotY, float ax, float ay)
    {
        float a = HDotX / ax;
        float b = HDotY / ay;
        float c = a * a + b * b + NDotH * NDotH;
        return 1.0 / (PI * ax * ay * c * c);
    }

    float Integrator::GTR1(float NDotH, float a)
    {
        if (a >= 1.0)
            return INV_PI;
        float a2 = a * a;
        float t = 1.0 + (a2 - 1.0) * NDotH * NDotH;
        return (a2 - 1.0) / (PI * log(a2) * t);
    }
    float Integrator::SmithG(float NDotV, float alphaG)
    {
        float a = alphaG * alphaG;
        float b = NDotV * NDotV;
        return (2.0 * NDotV) / (NDotV + sqrt(a + b - a * b));
    }
    float Integrator::SmithGAniso(float NDotV, float VDotX, float VDotY, float ax, float ay)
    {
        float a = VDotX * ax;
        float b = VDotY * ay;
        float c = NDotV;
        return (2.0 * NDotV) / (NDotV + sqrtf(a * a + b * b + c * c));
    }

    glm::vec3 Integrator::SampleGTR2Aniso(float ax, float ay, float r1, float r2)
    {
        float phi = r1 * TWO_PI;

        float sinPhi = ay * sin(phi);
        float cosPhi = ax * cos(phi);
        float tanTheta = sqrt(r2 / (1 - r2));

        return glm::vec3(tanTheta * cosPhi, tanTheta * sinPhi, 1.0);
    }

    glm::vec3 Integrator::SampleGTR1(float rgh, float r1, float r2)
    {
        float a = glm::max(0.001f, rgh);
        float a2 = a * a;

        float phi = r1 * TWO_PI;

        float cosTheta = sqrt((1.0 - pow(a2, 1.0 - r2)) / (1.0 - a2));
        float sinTheta = glm::clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        return glm::vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
    }
}
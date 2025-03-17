#include <cpu/integrator.hpp>

namespace scTracer::CPU
{
    glm::vec3 Integrator::DirectLight(Ray r, State state, bool isSurface)
    {
        glm::vec3 Ld{0.0f};
        glm::vec3 Li{0.0f};
        glm::vec3 scatterPos = state.fhp + float(EPS) * state.ffnormal;

        ScatterSampleRec scatterSample;

        // Analytic Lights
        {
            LightSampleRec lightSample;
            Light light;

            // Pick a light to sample
            int index = int(rand() * float(mScene->lights.size())) * 5;

            // Fetch light Data
            light.position = mScene->lights[index].position;
            light.emission = mScene->lights[index].emission;
            light.u = mScene->lights[index].u;
            light.v = mScene->lights[index].v;
            light.radius = mScene->lights[index].radius;
            light.area = mScene->lights[index].area;
            light.type = mScene->lights[index].type; // 0->Rect, 1->Sphere, 2->Distant

            SampleOneLight(light, scatterPos, lightSample);
            Li = lightSample.emission;

            if (dot(lightSample.direction, lightSample.normal) < 0.0) // Required for quad lights with single sided emission
            {
                Ray shadowRay = Ray(scatterPos, lightSample.direction);

                // If there are volumes in the scene then evaluate transmittance rather than a binary anyhit test

                // If there are no volumes in the scene then use a simple binary hit test
                bool inShadow = AnyHit(shadowRay, lightSample.dist - EPS);

                if (!inShadow)
                {
                    scatterSample.f = DisneyEval(state, -r.direction, state.ffnormal, lightSample.direction, scatterSample.pdf);

                    float misWeight = 1.0;
                    if (light.area > 0.0) // No MIS for distant light
                        misWeight = PowerHeuristic(lightSample.pdf, scatterSample.pdf);

                    if (scatterSample.pdf > 0.0)
                        Ld += misWeight * Li * scatterSample.f / lightSample.pdf;
                }
            }
        }

        return Ld;
    }
}
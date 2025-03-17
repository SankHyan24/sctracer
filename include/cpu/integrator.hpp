#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <GL/gl3w.h>
#include <glfw/glfw3.h>
// imgui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//
#include <config.hpp>
#include <core/scene.hpp>
#include <bvh/flattenbvh.hpp>
#include <utils.hpp>
#include <cpu/cpushader.hpp>
#include <cpu/tonemap.hpp>

namespace scTracer::CPU
{
    struct UniformVars
    {
        bool isCameraMoving;
        glm::vec3 randomVector;
        glm::vec2 resolution;

        // sampler2D accumTexture;
        // samplerBuffer BVH;
        // isamplerBuffer vertexIndicesTex;
        // samplerBuffer vertexTex;
        // samplerBuffer normalsTex;
        // samplerBuffer uvsTex;
        // sampler2D materialsTex;
        // sampler2D transformsTex;
        // sampler2D lightsTex;
        // sampler2DArray textureMapsArrayTex;

        glm::vec3 uniformLightCol;
        int numOfLights;
        int maxDepth;
        int topBVHIndex;
        int frameNum;
        float roughnessMollificationAmt;
    };

    class Integrator
    {
        // run this in window::renderer, after scene is prepared
    public:
        Integrator(Core::Scene &scene, float *canvas) : mScene(&scene), mCanvas(canvas)
        {
            _init();
        }
        ~Integrator()
        {
        }
        void render() // sample all pixels for one time
        {
            for (int y = 0; y < mCanvasHeight; y++)
                for (int x = 0; x < mCanvasWidth; x++)
                    renderPixel(x, y);
            mFrameNumber++;
        }
        void renderPixel(int x, int y)
        {
            __renderPixel(x, y);
        }

    protected:
        void _init()
        {
            mCanvasWidth = mScene->settings.image_width;
            mCanvasHeight = mScene->settings.image_height;

            uniforms.resolution = glm::vec2(mCanvasWidth, mCanvasHeight);
            uniforms.topBVHIndex = mScene->bvhFlattor.topLevelIndex;
            uniforms.maxDepth = mScene->settings.maxBounceDepth;
        }

    private:
        Core::Scene *mScene;
        UniformVars uniforms;
        float *mCanvas;
        int mCanvasWidth, mCanvasHeight;
        int mFrameNumber{0};

        void __renderPixel(int x, int y)
        {
            // prepare RNG
            uint32_t seed = static_cast<uint32_t>(y * mCanvasWidth + x) + mFrameNumber * 0x213;
            std::mt19937 pixel_rng(seed);
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            // prepare ray
            glm::vec2 coords = glm::vec2(
                (float)x / mCanvasWidth,
                (float)y / mCanvasHeight);

            float r1 = 2.0 * dist(pixel_rng);
            float r2 = 2.0 * dist(pixel_rng);
            glm::vec2 jitter;
            jitter.x = r1 < 1.0 ? sqrtf(r1) - 1.0 : 1.0 - sqrtf(2.0 - r1);
            jitter.y = r2 < 1.0 ? sqrtf(r2) - 1.0 : 1.0 - sqrtf(2.0 - r2);
            jitter /= uniforms.resolution * 0.5f;
            glm::vec2 d = (coords * 2.0f - 1.0f);

            float scale = tan(mScene->camera.mFov * 0.5f);

            d.y *= uniforms.resolution.y / uniforms.resolution.x * scale;
            d.x *= scale;
            glm::vec3 rayDir = glm::normalize(d.x * mScene->camera.mRight + d.y * mScene->camera.mUp + mScene->camera.mFront);

            glm::vec3 focalPoint = mScene->camera.mFocalDist * rayDir;
            float cam_r1 = rand() * TWO_PI;
            float cam_r2 = rand() * mScene->camera.mAperture;
            glm::vec3 randomAperturePos = (cos(cam_r1) * mScene->camera.mRight + sin(cam_r1) * mScene->camera.mUp) * sqrt(cam_r2);
            glm::vec3 finalRayDir = glm::normalize(focalPoint - randomAperturePos);

            Ray ray = Ray(mScene->camera.mPosition, finalRayDir);

            glm::vec4 pixelColor = __traceRay(ray);
            // glm::vec4 pixelColor {0.1,0.0,1,1};

            glm::vec4 color = pixelColor;

            {
                // color = CPU::toneMap(color);
                mCanvas[(y * mCanvasWidth + x) * 4 + 0] = color.r;
                mCanvas[(y * mCanvasWidth + x) * 4 + 1] = color.g;
                mCanvas[(y * mCanvasWidth + x) * 4 + 2] = color.b;
                mCanvas[(y * mCanvasWidth + x) * 4 + 3] = 1.0f;
            }
        }

        glm::vec4 __traceRay(Ray ray)
        {
            glm::vec3 radiance = glm::vec3(0.0f);
            float alpha = 1.0f;
            glm::vec3 throughput = glm::vec3(1.0f);

            CPU::State state;
            CPU::LightSampleRec lightSample;
            CPU::ScatterSampleRec scatterSample;

            bool inMedium = false;
            bool mediumSampled = false;
            bool surfaceScatter = false;

            glm::vec3 debuger = glm::vec3(0.0f);

            for (state.depth = 0;; state.depth++)
            {
                bool hit = ClosestHit(ray, state, lightSample, debuger);
                if (!hit)
                {
                    {
                    }
                    break;
                }
                GetMaterial(state, ray);

                if (state.isEmitter)
                {
                    float misWeight = 1.0;
                    if (state.depth > 0)
                        misWeight = PowerHeuristic(scatterSample.pdf, lightSample.pdf);
                    radiance += misWeight * lightSample.emission * throughput; // direct light from the emitter
                    break;
                }

                if (state.depth == uniforms.maxDepth)
                    break;

                {
                    surfaceScatter = true;
                    radiance += DirectLight(ray, state, true) * throughput;
                    scatterSample.f = DisneySample(state, -ray.direction, state.ffnormal, scatterSample.L, scatterSample.pdf);
                    if (scatterSample.pdf > 0.0)
                        throughput *= scatterSample.f / scatterSample.pdf;
                    else
                        break;
                }

                ray.direction = scatterSample.L;
                ray.origin = state.fhp + ray.direction * float(EPS);
            }
            return glm::vec4(radiance, alpha);
        }

        // directlight.cpp
        glm::vec3 Integrator::DirectLight(Ray r, State state, bool isSurface);
        // disney.cpp
        glm::vec3 Integrator::ToLocal(glm::vec3 X, glm::vec3 Y, glm::vec3 Z, glm::vec3 V);
        glm::vec3 Integrator::DisneyEval(State state, glm::vec3 V, glm::vec3 N, glm::vec3 L, float &pdf);
        void Integrator::TintColors(Material mat, float eta, float F0, glm::vec3 &Csheen, glm::vec3 &Cspec0);
        glm::vec3 Integrator::EvalDisneyDiffuse(Material mat, glm::vec3 Csheen, glm::vec3 V, glm::vec3 L, glm::vec3 H, float &pdf);
        glm::vec3 Integrator::DisneySample(State state, glm::vec3 V, glm::vec3 N, glm::vec3 &L, float &pdf);
        glm::vec3 Integrator::EvalMicrofacetReflection(Material mat, glm::vec3 V, glm::vec3 L, glm::vec3 H, glm::vec3 F, float &pdf);
        glm::vec3 Integrator::EvalMicrofacetRefraction(Material mat, float eta, glm::vec3 V, glm::vec3 L, glm::vec3 H, glm::vec3 F, float &pdf);
        glm::vec3 Integrator::ToWorld(glm::vec3 X, glm::vec3 Y, glm::vec3 Z, glm::vec3 V);
        glm::vec3 Integrator::EvalClearcoat(Material mat, glm::vec3 V, glm::vec3 L, glm::vec3 H, float &pdf);
        // hit.cpp
        void Integrator::GetMaterial(State &state, Ray r);
        bool Integrator::AnyHit(Ray r, float maxDist);

        bool Integrator::ClosestHit(Ray r, State &state, LightSampleRec &lightSample, glm::vec3 &debugger);
        // intersection.cpp
        float Integrator::SphereIntersect(float rad, glm::vec3 pos, Ray r);
        float Integrator::AABBIntersect(glm::vec3 minCorner, glm::vec3 maxCorner, Ray r);
        float Integrator::RectIntersect(glm::vec3 pos, glm::vec3 u, glm::vec3 v, glm::vec4 plane, Ray r);
        // sampling.cpp
        float Integrator::SchlickWeight(float u);
        glm::vec3 Integrator::UniformSampleHemisphere(float r1, float r2);
        void Integrator::Onb(glm::vec3 N, glm::vec3 &T, glm::vec3 B);
        void Integrator::SampleRectLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample);
        void Integrator::SampleSphereLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample);
        void Integrator::SampleDistantLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample);
        glm::vec3 Integrator::SampleHG(glm::vec3 V, float g, float r1, float r2);
        float Integrator::PhaseHG(float cosTheta, float g);
        void Integrator::SampleOneLight(Light light, glm::vec3 scatterPos, LightSampleRec &lightSample);
        float Integrator::DielectricFresnel(float cosThetaI, float eta);
        glm::vec3 Integrator::CosineSampleHemisphere(float r1, float r2);
        glm::vec3 Integrator::SampleGGXVNDF(glm::vec3 V, float ax, float ay, float r1, float r2);
        glm::vec3 Integrator::SampleGTR2Aniso(float ax, float ay, float r1, float r2);
        float Integrator::GTR2Aniso(float NDotH, float HDotX, float HDotY, float ax, float ay);
        float Integrator::PowerHeuristic(float a, float b);
        float Integrator::GTR1(float NDotH, float a);
        glm::vec3 Integrator::SampleGTR1(float rgh, float r1, float r2);
        float Integrator::SmithG(float NDotV, float alphaG);
        float Integrator::SmithGAniso(float NDotV, float VDotX, float VDotY, float ax, float ay);
    };
}
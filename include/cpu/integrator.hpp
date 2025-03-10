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
            mCanvasWidth = mScene->settings.image_width / 4;
            mCanvasHeight = mScene->settings.image_height / 4;

            uniforms.resolution = glm::vec2(mCanvasWidth, mCanvasHeight);
            uniforms.topBVHIndex = mScene->bvhFlattor.topLevelIndex;
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

            glm::vec4 color = pixelColor;

            {
                color = CPU::toneMap(color);
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
                }
                else
                {
                    radiance = glm::vec3(1.0);
                }
                break;
            }
            radiance = debuger;
            return glm::vec4(radiance, alpha);
        }

        bool ClosestHit(Ray r, State &state, LightSampleRec lightSample, glm::vec3 &debugger);
        float AABBIntersect(glm::vec3 minCorner, glm::vec3 maxCorner, Ray r);
    };

}
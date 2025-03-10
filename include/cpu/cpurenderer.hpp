#pragma once
#include <iostream>
#include <fstream>
#include <string>
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

#include <cpu/integrator.hpp>
namespace scTracer::CPU
{
    class CPURenderer
    {
    public:
        ~CPURenderer()
        {
            if (mHasCanvas)
            {
                delete[] mCanvas;
            }
        }

        void render2Canvas(Core::Scene &scene)
        {
            // prepare canvas

            if (!mHasCanvas)
            {
                mCanvas = new float[scene.settings.image_width * scene.settings.image_height * 4 / 16];
                std::cout << " New Canvas Created!" << std::endl;
                mHasCanvas = true;
            }
            else if (mCanvasHeight != scene.settings.image_height / 4 || mCanvasWidth != scene.settings.image_width / 4)
            {
                assert("Canvas size mismatch" && false);
            }
            mCanvasHeight = scene.settings.image_height / 4;
            mCanvasWidth = scene.settings.image_width / 4;
            numOfSamples = scene.settings.maxSamples;

            mScene = &scene;

            // render
            mIntegrator = new Integrator(scene, mCanvas);
            mIntegrator->render();

            // accumulate

            // dump2File("output.ppm");
            // dump to file if frame is max
            if (frameCounter == numOfSamples)
            {
                dump2File("output.ppm");
                frameCounter = 1;
            }
            else
            {
                frameCounter++;
            }
        }

        void dump2File(std::string filename)
        {
            // dump canvas to file as ppm format
            if (!mHasCanvas)
            {
                std::cerr << "No canvas to dump" << std::endl;
                exit(1);
            }
            std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::binary);
            ofs << "P6\n"
                << mCanvasWidth << " " << mCanvasHeight << "\n255\n";
            for (int i = 0; i < mCanvasWidth * mCanvasHeight; ++i)
            {
                ofs << (unsigned char)(std::min(float(0), mCanvas[i * 4 + 0] * 255))  // r
                    << (unsigned char)(std::min(float(0), mCanvas[i * 4 + 1] * 255))  // g
                    << (unsigned char)(std::min(float(0), mCanvas[i * 4 + 2] * 255)); // b
            }
            ofs.close();
            std::cerr << "Writing ppm file into:[" << filename << "]" << std::endl;
        }

        // Scene
        Core::Scene *mScene{nullptr};
        // canvas( a 2D array of pixels)
        bool mHasCanvas{false};
        float *mCanvas{nullptr};
        int mCanvasWidth{0}, mCanvasHeight{0};
        // integrator
        Integrator *mIntegrator;
        int numOfSamples{1}, frameCounter{1};
    };
}
#pragma once
#include <memory>
#include <iostream>

// opengl
#include <GL/gl3w.h>
#include <glfw/glfw3.h>

// imgui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// 
#include <config.hpp>
#include <core/scene.hpp>
#include <gpu/program.hpp>
#include <importer/pbrt/parser.hpp>

namespace scTracer::Window {

    enum class ShaderType {
        PathTracer,
        PathTracerLowResolution,
        ImageMap,
        ToneMap
    };

    struct RenderPipeline {
        void init() {}
        void load() {}
        void reload() {}
        GPU::Program* PathTracer;
        GPU::Program* PathTracerLowResolution;
        GPU::Program* ImageMap;
        GPU::Program* ToneMap;
    };

    class GLFWManager
    {
    public:
        GLFWManager() {
            if (!glfwInit())
            {
                std::cerr << "Failed to initialize GLFW" << std::endl;
                exit(1);
            }
            glfwWindowHint(GLFW_DEPTH_BITS, 24);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_DEPTH_BITS, 24);
        }

        ~GLFWManager() {
            glfwTerminate();
        }
    };

    class RenderGPU {
    public:
        RenderGPU() {
            std::cerr << "Render using [" << Config::LOG_GREEN << "GPU" << Config::LOG_RESET << "]" << std::endl;
            // auto load scenes
        }
        ~RenderGPU() = default;
        void render() {
            // render
        }

        // Scene
        Core::Scene* mScene{ nullptr };
        // Program
        RenderPipeline mRenderPipeline;
        // Context

        // Opengl buffer objects and textures for storing scene data on the GPU
        // FBOs
        // Render textures
        // Render resolution and window resolution
        // Variables to track rendering status
    private:

    };


}
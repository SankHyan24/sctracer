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
#include <importer/importer.hpp>

namespace scTracer::Window {

    enum class ShaderType {
        PathTracer,
        PathTracerLowResolution,
        ImageMap,
        ToneMap
    };

    struct RenderPipeline {
        void init() {
            vertexShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "vertex.glsl"), GL_VERTEX_SHADER);
            debuggerShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "debugger.glsl"), GL_FRAGMENT_SHADER);
            pathTracerShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "pathtracer.glsl"), GL_FRAGMENT_SHADER);
            pathTracerLowResolutionShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "pathtracer_low_resolution.glsl"), GL_FRAGMENT_SHADER);
            imageMapShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "imagemap.glsl"), GL_FRAGMENT_SHADER);
            toneMapShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "tonemap.glsl"), GL_FRAGMENT_SHADER);
        }
        void load() {
            Debugger = new scTracer::GPU::Program({ *vertexShader, *debuggerShader });
            PathTracer = new scTracer::GPU::Program({ *vertexShader, *pathTracerShader });
            PathTracerLowResolution = new scTracer::GPU::Program({ *vertexShader, *pathTracerLowResolutionShader });
            ImageMap = new scTracer::GPU::Program({ *vertexShader,*imageMapShader });
            ToneMap = new scTracer::GPU::Program({ *vertexShader, *toneMapShader });
        }
        void reload() {
            delete Debugger;
            delete PathTracer;
            delete PathTracerLowResolution;
            delete ImageMap;
            delete ToneMap;
            load();
        }
        ~RenderPipeline() {
            // delete shaders
            delete vertexShader;
            delete debuggerShader;
            delete pathTracerShader;
            delete pathTracerLowResolutionShader;
            delete imageMapShader;
            delete toneMapShader;
            // delete programs
            delete Debugger;
            delete PathTracer;
            delete PathTracerLowResolution;
            delete ImageMap;
            delete ToneMap;
        }
        // shaders
        GPU::Shader* vertexShader;
        GPU::Shader* debuggerShader;
        GPU::Shader* pathTracerShader;
        GPU::Shader* pathTracerLowResolutionShader;
        GPU::Shader* imageMapShader;
        GPU::Shader* toneMapShader;
        // programs
        GPU::Program* Debugger;
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
        void init() {
            mRenderPipeline.init();
            __loadSceneLists();
            __loadScene();
            // __loadShaders();
        }

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
        std::string mScenesRootPath{ Config::sceneFolder };
        std::vector<std::string> mSceneListPath;
        void __loadSceneLists() { // load all scenes
            for (const auto& entry : std::filesystem::directory_iterator(mScenesRootPath))
                if (entry.is_directory()) { // check if has .pbrt file in the folder
                    bool hasPbrt = false;
                    for (const auto& subEntry : std::filesystem::directory_iterator(entry.path()))
                        if (subEntry.is_regular_file() && subEntry.path().extension() == ".pbrt") {
                            hasPbrt = true;
                            break;
                        }
                    if (hasPbrt)
                        mSceneListPath.push_back(entry.path().string().substr(mScenesRootPath.size()));
                }
        }

        void __loadScene() {
            assert(mSceneListPath.size() > 0);
            __loadScene(mSceneListPath[0]);
        }

        void __loadScene(std::string sceneName) {
            std::string sceneFullPath = mScenesRootPath + sceneName;
            std::string scenePbrtName;
            for (const auto& entry : std::filesystem::directory_iterator(sceneFullPath))
                if (entry.is_regular_file() && entry.path().extension() == ".pbrt") {
                    scenePbrtName = entry.path().string();
                    break;
                }
            std::cerr << "Loading scene [" << scenePbrtName << "]" << std::endl;
            scTracer::Importer::Pbrt::pbrtParser pbrtScene(scenePbrtName);
        }

        void __loadShaders() {
            mRenderPipeline.load();

        }

    };


}
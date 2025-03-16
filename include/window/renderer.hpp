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
#include <window/quad.hpp>
#include <importer/importer.hpp>
#include <bvh/flattenbvh.hpp>
#include <utils.hpp>
#include <cpu/cpurenderer.hpp>

namespace scTracer::Window
{
    class Window;

    enum class ShaderType
    {
        PathTracer,
        PathTracerLowResolution,
        ImageMap,
        ToneMap
    };

    struct RenderPipeline
    {
        void init()
        {
            // vertex shader
            vertexShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "vertex.glsl"), GL_VERTEX_SHADER);
            debuggerVertShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "debugger.vert"), GL_VERTEX_SHADER);
            // fragment shaders
            debuggerFragShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "debugger.frag"), GL_FRAGMENT_SHADER);
            pathTracerShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "pathtracer.glsl"), GL_FRAGMENT_SHADER);
            pathTracerLowResolutionShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "pathtracer_low_resolution.glsl"), GL_FRAGMENT_SHADER);
            imageMapShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "imagemap.glsl"), GL_FRAGMENT_SHADER);
            toneMapShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "tonemap.glsl"), GL_FRAGMENT_SHADER);
        }
        void load()
        {
            // programs
            Debugger = new scTracer::GPU::Program({debuggerVertShader, debuggerFragShader});
            PathTracer = new scTracer::GPU::Program({vertexShader, pathTracerShader});
            PathTracerLowResolution = new scTracer::GPU::Program({vertexShader, pathTracerLowResolutionShader});
            ImageMap = new scTracer::GPU::Program({vertexShader, imageMapShader});
            ToneMap = new scTracer::GPU::Program({vertexShader, toneMapShader});
        }
        void reload()
        {
            delete Debugger;
            delete PathTracer;
            delete PathTracerLowResolution;
            delete ImageMap;
            delete ToneMap;
            load();
        }
        void reinit()
        {
            delete vertexShader;
            delete debuggerVertShader;
            delete debuggerFragShader;
            delete pathTracerShader;
            delete pathTracerLowResolutionShader;
            delete imageMapShader;
            delete toneMapShader;
            init();
        }
        ~RenderPipeline()
        {
            // delete shaders
            delete vertexShader;
            delete debuggerVertShader;
            delete debuggerFragShader;
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
        GPU::Shader *vertexShader{nullptr};
        GPU::Shader *debuggerFragShader{nullptr};
        GPU::Shader *debuggerVertShader{nullptr};
        GPU::Shader *pathTracerShader{nullptr};
        GPU::Shader *pathTracerLowResolutionShader{nullptr};
        GPU::Shader *imageMapShader{nullptr};
        GPU::Shader *toneMapShader{nullptr};
        // programs
        GPU::Program *Debugger{nullptr};
        GPU::Program *PathTracer{nullptr};
        GPU::Program *PathTracerLowResolution{nullptr};
        GPU::Program *ImageMap{nullptr};
        GPU::Program *ToneMap{nullptr};
    };

    struct RenderFrameBuffers
    {
        // for bvhs
        GLuint BVHBuffer;
        GLuint BVHTex;
        // for meshes
        GLuint vertexIndicesBuffer;
        GLuint vertexIndicesTex;
        GLuint vertexBuffer;
        GLuint vertexTex;
        GLuint normalBuffer;
        GLuint normalTex;
        GLuint uvBuffer;
        GLuint uvTex;
        //
        GLuint materialBuffer;
        GLuint materialTex;
        // instances
        GLuint transformsTex;
        // for lights
        GLuint lightsTex;
        // for textures
        GLuint textureMapsArrayTex;
        // for envmap
        GLuint envMapTex;
        GLuint envMapCDFTex;

        //
    };

    struct RenderFBOs
    {
        GLuint pathTracerFBO;
        GLuint pathTracerTexture;
        GLuint pathTracerLowResolutionFBO;
        GLuint pathTracerLowResolutionTexture;

        GLuint accumulationFBO;
        GLuint accumulationTexture;

        GLuint outputFBO;
        GLuint outputTexture[2];

        //
        GLuint debugFBO;
        GLuint debugTexture;
    };

    class GLFWManager
    {
    public:
        GLFWManager()
        {
            if (!glfwInit())
            {
                std::cerr << "Failed to initialize GLFW" << std::endl;
                exit(1);
            }
            glfwWindowHint(GLFW_DEPTH_BITS, 24);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // for renderdoc debug
        }

        ~GLFWManager()
        {
            glfwTerminate();
        }
    };

    class RenderGPU
    {
    public:
        RenderGPU();
        ~RenderGPU() = default;
        void init();
        void render();
        void show();
        void showCPU(CPU::CPURenderer *cpuRenderer);
        void update();
        void saveImage(std::string filename);

        Core::Scene *mScene{nullptr};
        // Render contexts
        RenderPipeline mRenderPipeline;
        RenderFrameBuffers mRenderFrameBuffers;
        RenderFBOs mRenderFBOs;
        // auxiliary
        Quad *mQuad{nullptr};

    private:
        // for rendering
        float previewScale{0.25f}; // which means 1/4 of the original resolution
        int numOfSamples{1}, frameCounter{1};
        int currentBuffer{0}; // 0 or 1
        // for window
        glm::ivec2 windowSize{Config::default_width, Config::default_height};
        glm::ivec2 lowResSize{windowSize.x * previewScale, windowSize.y *previewScale};
        std::string mScenesRootPath{Config::sceneFolder};
        std::vector<std::string> mSceneListPath;
        void __loadSceneLists();
        void __loadScene();
        void __loadScene(std::string sceneName);
        void __loadShaders();
        void __initGPUDateBuffers();
        void __initFBOs();
        void __captureFrame(unsigned char *buffer);
        friend class Window;
    };

}
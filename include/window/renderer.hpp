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
        RenderPipeline() = default;
        void init();
        void load();
        void reload();
        void reinit();
        ~RenderPipeline();
        // shaders
        GPU::Shader *vertexShader{nullptr};
        GPU::Shader *debuggerFragShader{nullptr};
        GPU::Shader *debuggerVertShader{nullptr};
        GPU::Shader *pathTracerShader{nullptr};
        GPU::Shader *pathTracerLowResolutionShader{nullptr};
        GPU::Shader *imageMapShader{nullptr};
        GPU::Shader *toneMapShader{nullptr};
        GPU::Shader *accumulateShader{nullptr};
        // programs
        GPU::Program *Debugger{nullptr};
        GPU::Program *PathTracer{nullptr};
        GPU::Program *PathTracerLowResolution{nullptr};
        GPU::Program *ImageMap{nullptr};
        GPU::Program *ToneMap{nullptr};
        GPU::Program *Accumulate{nullptr};
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

        GLuint accumulatedFBO;
        GLuint accumulatedTexture;

        GLuint outputFBO;
        GLuint outputTexture[2];

        //
        GLuint debugFBO;
        GLuint debugTexture;
    };

    class GLFWManager
    {
    public:
        GLFWManager();
        ~GLFWManager();
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
        void saveEXR(std::string filename);

        Core::Scene *mScene{nullptr};
        bool shaderNeedReload{false};
        // Render contexts
        RenderPipeline mRenderPipeline;
        RenderFrameBuffers mRenderFrameBuffers;
        RenderFBOs mRenderFBOs;
        // auxiliary
        Quad *mQuad{nullptr};

    private:
        // for rendering
        double timeBegin{0.0f}, timeThisFrame{0.0f};
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
        void __captureFrame(float *buffer);
        friend class Window;
    };

}
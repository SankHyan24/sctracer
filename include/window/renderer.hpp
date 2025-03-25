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
        void freeAllTex()
        {
            glDeleteTextures(1, &BVHTex);
            glDeleteTextures(1, &vertexIndicesTex);
            glDeleteTextures(1, &vertexTex);
            glDeleteTextures(1, &normalTex);
            glDeleteTextures(1, &uvTex);
            glDeleteTextures(1, &materialTex);
            glDeleteTextures(1, &transformsTex);
            glDeleteTextures(1, &lightsTex);
            glDeleteTextures(1, &textureMapsArrayTex);
            glDeleteTextures(1, &envMapTex);
            glDeleteTextures(1, &envMapCDFTex);
        }

        void freeAllBuffers()
        {
            glDeleteBuffers(1, &BVHBuffer);
            glDeleteBuffers(1, &vertexIndicesBuffer);
            glDeleteBuffers(1, &vertexBuffer);
            glDeleteBuffers(1, &normalBuffer);
            glDeleteBuffers(1, &uvBuffer);
            glDeleteBuffers(1, &materialBuffer);
        }
    };

    struct RenderFBOs
    {
        GLuint pathTracerFBO;
        GLuint pathTracerTexture;
        GLuint pathTracerLowResolutionFBO;
        GLuint CPUrenderTexture;

        GLuint accumulationFBO;
        GLuint accumulationTexture;

        GLuint accumulatedFBO;
        GLuint accumulatedTexture;

        GLuint outputFBO;
        GLuint outputTexture[2];

        //
        GLuint debugFBO;
        GLuint debugTexture;

        void freeAllTex()
        {
            glDeleteTextures(1, &pathTracerTexture);
            glDeleteTextures(1, &CPUrenderTexture);
            glDeleteTextures(1, &accumulationTexture);
            glDeleteTextures(1, &accumulatedTexture);
            glDeleteTextures(1, &outputTexture[0]);
            glDeleteTextures(1, &outputTexture[1]);
            glDeleteTextures(1, &debugTexture);
        }

        void freeAllFramebuffers()
        {
            glDeleteFramebuffers(1, &pathTracerFBO);
            glDeleteFramebuffers(1, &pathTracerLowResolutionFBO);
            glDeleteFramebuffers(1, &accumulationFBO);
            glDeleteFramebuffers(1, &accumulatedFBO);
            glDeleteFramebuffers(1, &outputFBO);
            glDeleteFramebuffers(1, &debugFBO);
        }
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
        RenderGPU(bool useGPU);
        ~RenderGPU() = default;
        void init();
        void reInitScene(std::string sceneName);
        void freeAll();
        void render();
        void show();
        void showCPU(CPU::CPURenderer *cpuRenderer);
        void update();
        void saveImage(std::string filename);
        void saveEXR(std::string filename);

        Core::Scene *mScene{nullptr};
        bool shaderNeedReload{false};
        std::string mScenesRootPath{Config::sceneFolder};
        std::vector<std::string> mPbrtSceneListPath;
        std::vector<std::string> mMayaSceneListPath;
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
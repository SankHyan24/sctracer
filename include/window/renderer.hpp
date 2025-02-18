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
        GLuint transformTex;
        // for lights
        GLuint lightTex;
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
            glfwWindowHint(GLFW_DEPTH_BITS, 24);
        }

        ~GLFWManager()
        {
            glfwTerminate();
        }
    };

    class RenderGPU
    {
    public:
        RenderGPU()
        {
            std::cerr << "Render using [" << Config::LOG_GREEN << "GPU" << Config::LOG_RESET << "]" << std::endl;
            // auto load scenes
        }
        ~RenderGPU() = default;
        void init()
        {
            __loadSceneLists();
            __loadScene();
            __initGPUDateBuffers();
            __initFBOs();
            __loadShaders();
            mQuad = new Quad();
            Utils::glUtils::checkError("RenderGPU::init");
        }
        void render()
        {
            if (!mScene->isDirty() && mScene->settings.maxSamples != -1 && numOfSamples >= mScene->settings.maxSamples)
                return;
            glActiveTexture(GL_TEXTURE0);
            {
                glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.pathTracerFBO);
                glViewport(0, 0, windowSize.x, windowSize.y);
                glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulationTexture);
                mQuad->draw(mRenderPipeline.PathTracer);

                glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulationFBO);
                glViewport(0, 0, windowSize.x, windowSize.y);
                glBindTexture(GL_TEXTURE_2D, mRenderFBOs.pathTracerTexture);
                mQuad->draw(mRenderPipeline.ImageMap);

                glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.outputFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.outputTexture[currentBuffer], 0);
                glViewport(0, 0, windowSize.x, windowSize.y);
                glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulationTexture);
                mQuad->draw(mRenderPipeline.ToneMap);
            }

            {
                // render debug
                glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.debugFBO);
                glViewport(0, 0, windowSize.x, windowSize.y);
                glBindTexture(GL_TEXTURE_2D, mRenderFBOs.debugTexture);
                mQuad->draw(mRenderPipeline.Debugger);
            }

            Utils::glUtils::checkError("RenderGPU::render");
        }
        void show()
        { // to screen
            glActiveTexture(GL_TEXTURE0);
            {
                // glBindTexture(GL_TEXTURE_2D, mRenderFBOs.outputTexture[1 - currentBuffer]);
                glBindTexture(GL_TEXTURE_2D, mRenderFBOs.debugTexture);
                mQuad->draw(mRenderPipeline.ImageMap);
            }
            Utils::glUtils::checkError("RenderGPU::show");
        }
        void RenderGPU::update()
        {
            if (!mScene->isDirty() && mScene->settings.maxSamples != -1 && numOfSamples >= mScene->settings.maxSamples)
                return;
            if (mScene->instancesDirty)
            {
                mScene->instancesDirty = false;
                // Update transforms
                glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.transformTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(glm::mat4) / sizeof(float) / 4 * mScene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->transforms[0]);

                glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.materialTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(Core::Material) / sizeof(float) / 4 * mScene->materials.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->materialDatas[0]);

                // int index = mScene->bvhFlattor.topLevelIndex;
                // int offset = sizeof(BVH::BVHFlattor::flattenedNodes) * index;
                // int size = sizeof(BVH::BVHFlattor::flattenedNodes) * (mScene->bvhFlattor.flattenedNodes.size() - index);
                // glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHBuffer);
                // glBufferSubData(GL_TEXTURE_BUFFER, offset, size, &mScene->bvhFlattor.flattenedNodes[index]);
            }

            if (mScene->envMapDirty)
            {
                mScene->envMapDirty = false;
                // Update envmap
                // TODO
            }

            if (mScene->isDirty())
            {
                numOfSamples = 1; // reset samples
                frameCounter = 1;
                mScene->dirty = false;
                // clear accumulation buffer
                glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulationFBO);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            else
            {
                frameCounter++;
                numOfSamples++;
                currentBuffer = 1 - currentBuffer;
            }

            // Update uniforms
            mRenderPipeline.PathTracer->Use();
            auto thisProgram = mRenderPipeline.PathTracer->get();
            mScene->camera.rotateByZero();
            glUniform3f(glGetUniformLocation(thisProgram, "camera.position"), mScene->camera.mPosition.x, mScene->camera.mPosition.y, mScene->camera.mPosition.z);
            glUniform3f(glGetUniformLocation(thisProgram, "camera.right"), mScene->camera.mRight.x, mScene->camera.mRight.y, mScene->camera.mRight.z);
            glUniform3f(glGetUniformLocation(thisProgram, "camera.up"), mScene->camera.mUp.x, mScene->camera.mUp.y, mScene->camera.mUp.z);
            glUniform3f(glGetUniformLocation(thisProgram, "camera.front"), mScene->camera.mFront.x, mScene->camera.mFront.y, mScene->camera.mFront.z);
            glUniform1f(glGetUniformLocation(thisProgram, "camera.fov"), mScene->camera.mFov);
            glUniform1f(glGetUniformLocation(thisProgram, "camera.focalDist"), mScene->camera.mFocalDist);
            glUniform1f(glGetUniformLocation(thisProgram, "camera.aperture"), mScene->camera.mAperture);
            glUniform1i(glGetUniformLocation(thisProgram, "maxDepth"), mScene->settings.maxBounceDepth);
            glUniform1i(glGetUniformLocation(thisProgram, "frameNum"), frameCounter);
            mRenderPipeline.PathTracer->StopUsing();

            mRenderPipeline.PathTracerLowResolution->Use();
            thisProgram = mRenderPipeline.PathTracerLowResolution->get();
            mRenderPipeline.PathTracerLowResolution->StopUsing();

            mRenderPipeline.ToneMap->Use();
            thisProgram = mRenderPipeline.ToneMap->get();
            glUniform1f(glGetUniformLocation(thisProgram, "invSampleCounter"), 1.0f / (numOfSamples));
            mRenderPipeline.ToneMap->StopUsing();

            Utils::glUtils::checkError("RenderGPU::update");
        }
        // Scene
        Core::Scene *mScene{nullptr};
        // Program
        RenderPipeline mRenderPipeline;

        // Context
        RenderFrameBuffers mRenderFrameBuffers;
        RenderFBOs mRenderFBOs;
        Quad *mQuad{nullptr};
        // Opengl buffer objects and textures for storing scene data on the GPU
        // FBOs
        // Render textures
        // Render resolution and window resolution
        // Variables to track rendering status
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
        void __loadSceneLists()
        { // load all scenes
            for (const auto &entry : std::filesystem::directory_iterator(mScenesRootPath))
                if (entry.is_directory())
                { // check if has .pbrt file in the folder
                    bool hasPbrt = false;
                    for (const auto &subEntry : std::filesystem::directory_iterator(entry.path()))
                        if (subEntry.is_regular_file() && subEntry.path().extension() == ".pbrt")
                        {
                            hasPbrt = true;
                            break;
                        }
                    if (hasPbrt)
                        mSceneListPath.push_back(entry.path().string().substr(mScenesRootPath.size()));
                }
        }
        void __loadScene()
        {
            assert(mSceneListPath.size() > 0);
            __loadScene(mSceneListPath[0]);
        }
        void __loadScene(std::string sceneName)
        {
            // load and process scene
            std::string sceneFullPath = mScenesRootPath + sceneName;
            std::string scenePbrtName;
            for (const auto &entry : std::filesystem::directory_iterator(sceneFullPath))
                if (entry.is_regular_file() && entry.path().extension() == ".pbrt")
                {
                    scenePbrtName = entry.path().string();
                    break;
                }
            std::cerr << "Loading scene [" << scenePbrtName << "]" << std::endl;
            mScene = scTracer::Importer::Pbrt::pbrtParser::parse(scenePbrtName);
            if (!mScene->isInitialized())
                mScene->processScene();
        }
        void __loadShaders()
        { // reload = load
            mRenderPipeline.reinit();
            mRenderPipeline.reload();

            // Setup shader uniforms
            mRenderPipeline.PathTracer->Use();
            GLuint thisProgram = mRenderPipeline.PathTracer->get();

            // env map uniform here(not implemented yet)

            glUniform1i(glGetUniformLocation(thisProgram, "topBVHIndex"), mScene->bvhFlattor.topLevelIndex);
            glUniform2f(glGetUniformLocation(thisProgram, "resolution"), float(windowSize.x), float(windowSize.y));
            glUniform1i(glGetUniformLocation(thisProgram, "numOfLights"), mScene->lights.size());
            glUniform1i(glGetUniformLocation(thisProgram, "accumTexture"), 0);
            glUniform1i(glGetUniformLocation(thisProgram, "BVH"), 1);
            glUniform1i(glGetUniformLocation(thisProgram, "vertexIndicesTex"), 2);
            glUniform1i(glGetUniformLocation(thisProgram, "verticesTex"), 3);
            glUniform1i(glGetUniformLocation(thisProgram, "normalsTex"), 4);
            glUniform1i(glGetUniformLocation(thisProgram, "uvsTex"), 5);
            glUniform1i(glGetUniformLocation(thisProgram, "materialsTex"), 6);
            glUniform1i(glGetUniformLocation(thisProgram, "transformsTex"), 7);
            glUniform1i(glGetUniformLocation(thisProgram, "lightsTex"), 8);
            glUniform1i(glGetUniformLocation(thisProgram, "textureMapsArrayTex"), 9);
            mRenderPipeline.PathTracer->StopUsing();

            mRenderPipeline.PathTracerLowResolution->Use();
            thisProgram = mRenderPipeline.PathTracerLowResolution->get();
            glUniform1i(glGetUniformLocation(thisProgram, "topBVHIndex"), mScene->bvhFlattor.topLevelIndex);
            glUniform2f(glGetUniformLocation(thisProgram, "resolution"), float(lowResSize.x), float(lowResSize.y));
            glUniform1i(glGetUniformLocation(thisProgram, "numOfLights"), mScene->lights.size());
            glUniform1i(glGetUniformLocation(thisProgram, "accumTexture"), 0);
            glUniform1i(glGetUniformLocation(thisProgram, "BVH"), 1);
            glUniform1i(glGetUniformLocation(thisProgram, "vertexIndicesTex"), 2);
            glUniform1i(glGetUniformLocation(thisProgram, "verticesTex"), 3);
            glUniform1i(glGetUniformLocation(thisProgram, "normalsTex"), 4);
            glUniform1i(glGetUniformLocation(thisProgram, "uvsTex"), 5);
            glUniform1i(glGetUniformLocation(thisProgram, "materialsTex"), 6);
            glUniform1i(glGetUniformLocation(thisProgram, "transformsTex"), 7);
            glUniform1i(glGetUniformLocation(thisProgram, "lightsTex"), 8);
            glUniform1i(glGetUniformLocation(thisProgram, "textureMapsArrayTex"), 9);
            mRenderPipeline.PathTracerLowResolution->StopUsing();
        }
        void __initGPUDateBuffers()
        {
            std::cerr << Config::LOG_MAGENTA << "Init GPU Buffers" << Config::LOG_RESET;

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            // Create buffer and texture for BVH
            glGenBuffers(1, &mRenderFrameBuffers.BVHBuffer);
            glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHBuffer);
            glBufferData(GL_TEXTURE_BUFFER, sizeof(BVH::BVHFlattor::flattenedNodes) * mScene->bvhFlattor.flattenedNodes.size(), &mScene->bvhFlattor.flattenedNodes[0], GL_STATIC_DRAW);
            glGenTextures(1, &mRenderFrameBuffers.BVHTex);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHTex);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, mRenderFrameBuffers.BVHBuffer);
            // Create buffer and texture for vertex indices
            glGenBuffers(1, &mRenderFrameBuffers.vertexIndicesBuffer);
            glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexIndicesBuffer);
            glBufferData(GL_TEXTURE_BUFFER, sizeof(int) * mScene->sceneTriIndices.size(), &mScene->sceneTriIndices[0], GL_STATIC_DRAW);
            glGenTextures(1, &mRenderFrameBuffers.vertexIndicesTex);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexIndicesTex);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, mRenderFrameBuffers.vertexIndicesBuffer);
            // Create buffer and texture for vertices
            glGenBuffers(1, &mRenderFrameBuffers.vertexBuffer);
            glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexBuffer);
            glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec3) * mScene->sceneVertices.size(), &mScene->sceneVertices[0], GL_STATIC_DRAW);
            glGenTextures(1, &mRenderFrameBuffers.vertexTex);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexTex);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, mRenderFrameBuffers.vertexBuffer);
            // Create buffer and texture for normals
            glGenBuffers(1, &mRenderFrameBuffers.normalBuffer);
            glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.normalBuffer);
            glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec3) * mScene->sceneNormals.size(), &mScene->sceneNormals[0], GL_STATIC_DRAW);
            glGenTextures(1, &mRenderFrameBuffers.normalTex);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.normalTex);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, mRenderFrameBuffers.normalBuffer);
            // Create buffer and texture for uvs
            glGenBuffers(1, &mRenderFrameBuffers.uvBuffer);
            glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.uvBuffer);
            glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec2) * mScene->sceneMeshUvs.size(), &mScene->sceneMeshUvs[0], GL_STATIC_DRAW);
            glGenTextures(1, &mRenderFrameBuffers.uvTex);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.uvTex);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, mRenderFrameBuffers.uvBuffer);
            // Create buffer and texture for materials
            glGenTextures(1, &mRenderFrameBuffers.materialTex);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.materialTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(Core::Material) / sizeof(float) / 4 * mScene->materials.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->materialDatas[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            // Create buffer and texture for transforms
            glGenTextures(1, &mRenderFrameBuffers.transformTex);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.transformTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(glm::mat4) / sizeof(float) / 4 * mScene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->transforms[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            // Create texture for lights
            if (mScene->lights.size() > 0)
            {
                glGenTextures(1, &mRenderFrameBuffers.lightTex);
                glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.lightTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, sizeof(Core::Light) / sizeof(float) / 3 * mScene->lights.size(), 1, 0, GL_RGB, GL_FLOAT, &mScene->lights[0]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            // Create texture for scene textures
            // envmap
            // TODO
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHTex);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexIndicesTex);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexTex);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.normalTex);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.uvTex);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.materialTex);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.transformTex);
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.lightTex);

            // glBindTexture(GL_TEXTURE_2D_ARRAY, mRenderFrameBuffers.textureMapsArrayTex);
            // glActiveTexture(GL_TEXTURE9);
            // glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.envMapTex);
            // glActiveTexture(GL_TEXTURE10);
            // glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.envMapCDFTex);

            std::cerr << " ... " << Config::LOG_GREEN << "Done!" << Config::LOG_RESET << std::endl;
        }
        void __initFBOs()
        {
            std::cerr << Config::LOG_MAGENTA << "Init FBOs" << Config::LOG_RESET;
            numOfSamples = 1;
            frameCounter = 1;
            currentBuffer = 0;
            windowSize = glm::ivec2(windowSize.x, windowSize.y);
            lowResSize = glm::ivec2(windowSize.x * previewScale, windowSize.y * previewScale);

            glGenFramebuffers(1, &mRenderFBOs.pathTracerFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.pathTracerFBO);
            glGenTextures(1, &mRenderFBOs.pathTracerTexture);
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.pathTracerTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.pathTracerTexture, 0);

            glGenFramebuffers(1, &mRenderFBOs.pathTracerLowResolutionFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.pathTracerLowResolutionFBO);
            glGenTextures(1, &mRenderFBOs.pathTracerLowResolutionTexture);
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.pathTracerLowResolutionTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, lowResSize.x, lowResSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.pathTracerLowResolutionTexture, 0);

            glGenFramebuffers(1, &mRenderFBOs.accumulationFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulationFBO);
            glGenTextures(1, &mRenderFBOs.accumulationTexture);
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulationTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.accumulationTexture, 0);

            glGenFramebuffers(1, &mRenderFBOs.outputFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.outputFBO);
            for (int i = 0; i < 2; i++)
            {
                glGenTextures(1, &mRenderFBOs.outputTexture[i]);
                glBindTexture(GL_TEXTURE_2D, mRenderFBOs.outputTexture[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.outputTexture[currentBuffer], 0);

            // for debug
            glGenFramebuffers(1, &mRenderFBOs.debugFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.debugFBO);
            glGenTextures(1, &mRenderFBOs.debugTexture);
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.debugTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.debugTexture, 0);

            std::cerr << " ... " << Config::LOG_GREEN << "Done!" << Config::LOG_RESET << std::endl;
        }
        friend class Window;
    };

}
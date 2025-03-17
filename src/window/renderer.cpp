#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include <filesystem>
#include <window/renderer.hpp>

namespace scTracer::Window
{
    void RenderPipeline::init()
    {
        vertexShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "vertex.glsl"), GL_VERTEX_SHADER);
        pathTracerShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "pathtracer.glsl"), GL_FRAGMENT_SHADER);
        pathTracerLowResolutionShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "pathtracer_low_resolution.glsl"), GL_FRAGMENT_SHADER);
        imageMapShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "imagemap.glsl"), GL_FRAGMENT_SHADER);
        accumulateShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "accumulate.glsl"), GL_FRAGMENT_SHADER);
        toneMapShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "tonemap.glsl"), GL_FRAGMENT_SHADER);
        // debug shaders
        debuggerVertShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "debugger.vert"), GL_VERTEX_SHADER);
        debuggerFragShader = new scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "debugger.frag"), GL_FRAGMENT_SHADER);
    }

    void RenderPipeline::load()
    {
        // programs
        Debugger = new scTracer::GPU::Program({debuggerVertShader, debuggerFragShader});
        PathTracer = new scTracer::GPU::Program({vertexShader, pathTracerShader});
        PathTracerLowResolution = new scTracer::GPU::Program({vertexShader, pathTracerLowResolutionShader});
        ImageMap = new scTracer::GPU::Program({vertexShader, imageMapShader});
        Accumulate = new scTracer::GPU::Program({vertexShader, accumulateShader});
        ToneMap = new scTracer::GPU::Program({vertexShader, toneMapShader});
    }

    void RenderPipeline::reload()
    {
        delete Debugger;
        delete PathTracer;
        delete PathTracerLowResolution;
        delete ImageMap;
        delete Accumulate;
        delete ToneMap;
        load();
    }

    void RenderPipeline::reinit()
    {
        delete vertexShader;
        delete debuggerVertShader;
        delete debuggerFragShader;
        delete pathTracerShader;
        delete pathTracerLowResolutionShader;
        delete imageMapShader;
        delete accumulateShader;
        delete toneMapShader;
        init();
    }

    RenderPipeline::~RenderPipeline()
    {
        // delete shaders
        delete vertexShader;
        delete debuggerVertShader;
        delete debuggerFragShader;
        delete pathTracerShader;
        delete pathTracerLowResolutionShader;
        delete imageMapShader;
        delete accumulateShader;
        delete toneMapShader;
        // delete programs
        delete Debugger;
        delete PathTracer;
        delete PathTracerLowResolution;
        delete ImageMap;
        delete Accumulate;
        delete ToneMap;
    }

    GLFWManager::GLFWManager()
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(1);
        }
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // for renderdoc debug
    }

    GLFWManager::~GLFWManager()
    {
        glfwTerminate();
    }

    RenderGPU::RenderGPU(bool useGPU)
    {
        std::cerr << "Render using [" << Config::LOG_GREEN << (useGPU ? "GPU" : "CPU") << Config::LOG_RESET << "]" << std::endl;
    }

    void RenderGPU::init()
    {
        // auto load scenes
        __loadSceneLists();
        __loadScene();
        __initGPUDateBuffers();
        mQuad = new Quad();
        __initFBOs();
        __loadShaders();
        Utils::glUtils::checkError("RenderGPU::init");
    }

    void RenderGPU::render()
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

            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulatedFBO);
            glViewport(0, 0, windowSize.x, windowSize.y);
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulationTexture);
            mQuad->draw(mRenderPipeline.Accumulate);

            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.outputFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.outputTexture[currentBuffer], 0);
            glViewport(0, 0, windowSize.x, windowSize.y);
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulatedTexture);
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

    void RenderGPU::saveImage(std::string filename)
    {
        int width = windowSize.x;
        int height = windowSize.y;
        int size = width * height * 4;
        unsigned char *buffer = new unsigned char[size];
        __captureFrame(buffer);
        // save to file
        std::string fullPath = Config::outputFolder + filename;
        stbi_flip_vertically_on_write(1);
        stbi_write_png(fullPath.c_str(), width, height, 4, buffer, width * 4);
        delete[] buffer;
    }

    void RenderGPU::saveEXR(std::string filename)
    {
        int width = windowSize.x;
        int height = windowSize.y;
        int size = width * height * 4;
        float *buffer = new float[size];
        __captureFrame(buffer);
        // save to file
        std::string fullPath = Config::outputFolder + filename;

        EXRHeader header;
        InitEXRHeader(&header);
        EXRImage exrImage;
        InitEXRImage(&exrImage);

        std::vector<float> r(width * height);
        std::vector<float> g(width * height);
        std::vector<float> b(width * height);
        std::vector<float> a(width * height);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int index = i * width + j;
                int index_height_fliped = (height - i - 1) * width + j;
                r[index_height_fliped] = buffer[index * 4 + 0];
                g[index_height_fliped] = buffer[index * 4 + 1];
                b[index_height_fliped] = buffer[index * 4 + 2];
                a[index_height_fliped] = buffer[index * 4 + 3];
            }
        }
        float *imagePtrs[4] = {b.data(), g.data(), r.data(), a.data()};

        exrImage.num_channels = 4;
        exrImage.images = (unsigned char **)imagePtrs;
        exrImage.width = width;
        exrImage.height = height;
        EXRChannelInfo channelInfos[4];
        header.num_channels = 4;
        header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
        strncpy(header.channels[0].name, "B", 255);
        header.channels[0].name[strlen("B")] = '\0';
        strncpy(header.channels[1].name, "G", 255);
        header.channels[1].name[strlen("G")] = '\0';
        strncpy(header.channels[2].name, "R", 255);
        header.channels[2].name[strlen("R")] = '\0';
        strncpy(header.channels[3].name, "A", 255);
        header.channels[3].name[strlen("A")] = '\0';

        header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        for (int i = 0; i < header.num_channels; i++)
        {
            header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;          // pixel type of input image
            header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
        }

        const char *err = NULL; // or nullptr in C++11 or later.
        int ret = SaveEXRImageToFile(&exrImage, &header, fullPath.c_str(), &err);
        if (ret != TINYEXR_SUCCESS)
        {
            fprintf(stderr, "Save EXR err: %s\n", err);
            FreeEXRErrorMessage(err); // free's buffer for an error message
            exit(ret);
        }
        std::cout << "Saved exr file. [" << fullPath << "]" << std::endl;

        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);
        delete[] buffer;
    }

    void RenderGPU::show()
    { // to screen
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mRenderFBOs.outputTexture[1 - currentBuffer]);
        mQuad->draw(mRenderPipeline.ImageMap);
        Utils::glUtils::checkError("RenderGPU::show");
    }

    void RenderGPU::showCPU(CPU::CPURenderer *cpuRenderer)
    {
        float *canvas = cpuRenderer->mCanvas;
        int height = cpuRenderer->mCanvasHeight;
        int width = cpuRenderer->mCanvasWidth;
        glActiveTexture(GL_TEXTURE0);
        {
            glBindTexture(GL_TEXTURE_2D, mRenderFBOs.CPUrenderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, canvas);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            mQuad->draw(mRenderPipeline.ImageMap);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        Utils::glUtils::checkError("RenderGPU::showCPU");
    }

    void RenderGPU::update()
    {
        if (!mScene->isDirty() && mScene->settings.maxSamples != -1 && numOfSamples >= mScene->settings.maxSamples)
            return;
        if (shaderNeedReload)
        {
            shaderNeedReload = false;
            __loadShaders();
            std::cerr << Config::LOG_BLUE << "Shaders Reloaded" << Config::LOG_RESET << std::endl;
        }
        if (mScene->instancesDirty)
        {
            mScene->instancesDirty = false;
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.transformsTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(glm::mat4) / sizeof(float) / 4 * mScene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->transforms[0]);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.materialTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(Core::Material) / sizeof(float) / 4 * mScene->materialDatas.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->materialDatas[0]);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.lightsTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, sizeof(Core::Light) / sizeof(float) / 3 * mScene->lights.size(), 1, 0, GL_RGB, GL_FLOAT, &mScene->lights[0]);
            int index = mScene->bvhFlattor.topLevelIndex;
            int offset = sizeof(BVH::BVHFlattor::FlatNode) * index;
            int size = sizeof(BVH::BVHFlattor::FlatNode) * (mScene->bvhFlattor.flattenedNodes.size() - index);
            glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHBuffer);
            glBufferSubData(GL_TEXTURE_BUFFER, offset, size, &mScene->bvhFlattor.flattenedNodes[index]);
            std::cerr << Config::LOG_BLUE << "Instances Reloaded" << Config::LOG_RESET << std::endl;
        }

        if (mScene->envMapDirty)
        {
            mScene->envMapDirty = false;
            // Update envmap
            // TODO
        }

        if (mScene->isDirty())
        {
            timeBegin = glfwGetTime();
            numOfSamples = 1; // reset samples
            frameCounter = 1;
            mScene->dirty = false;
            // clear accumulation buffer
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulationFBO);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            timeThisFrame = glfwGetTime();
            frameCounter++;
            numOfSamples++;
            currentBuffer = 1 - currentBuffer;
            glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.outputFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.outputTexture[currentBuffer], 0);
        }

        // Update uniforms
        mRenderPipeline.PathTracer->Use();
        auto thisProgram = mRenderPipeline.PathTracer->get();
        // mScene->camera.rotateByZero();
        glUniform3f(glGetUniformLocation(thisProgram, "camera.up"), mScene->camera.mUp.x, mScene->camera.mUp.y, mScene->camera.mUp.z);
        glUniform3f(glGetUniformLocation(thisProgram, "camera.right"), mScene->camera.mRight.x, mScene->camera.mRight.y, mScene->camera.mRight.z);
        glUniform3f(glGetUniformLocation(thisProgram, "camera.forward"), mScene->camera.mFront.x, mScene->camera.mFront.y, mScene->camera.mFront.z);
        glUniform3f(glGetUniformLocation(thisProgram, "camera.position"), mScene->camera.mPosition.x, mScene->camera.mPosition.y, mScene->camera.mPosition.z);
        glUniform1f(glGetUniformLocation(thisProgram, "camera.fov"), mScene->camera.mFov);
        glUniform1f(glGetUniformLocation(thisProgram, "camera.focalDist"), mScene->camera.mFocalDist);
        glUniform1f(glGetUniformLocation(thisProgram, "camera.aperture"), mScene->camera.mAperture);
        glUniform1i(glGetUniformLocation(thisProgram, "maxDepth"), mScene->settings.maxBounceDepth);
        glUniform1i(glGetUniformLocation(thisProgram, "frameNum"), frameCounter);
        mRenderPipeline.PathTracer->StopUsing();

        mRenderPipeline.PathTracerLowResolution->Use();
        thisProgram = mRenderPipeline.PathTracerLowResolution->get();
        mRenderPipeline.PathTracerLowResolution->StopUsing();

        mRenderPipeline.Accumulate->Use();
        thisProgram = mRenderPipeline.Accumulate->get();
        glUniform1f(glGetUniformLocation(thisProgram, "invSampleCounter"), 1.0f / (numOfSamples));
        mRenderPipeline.Accumulate->StopUsing();

        Utils::glUtils::checkError("RenderGPU::update");
    }

    void RenderGPU::__loadSceneLists()
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
                    mPbrtSceneListPath.push_back(entry.path().string().substr(mScenesRootPath.size()));
            }
    }

    void RenderGPU::__loadScene(std::string sceneName)
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
        Utils::glUtils::checkError("RenderGPU::__loadScene");
    }

    void RenderGPU::__loadScene()
    {
        assert(mPbrtSceneListPath.size() > 0);
        __loadScene(mPbrtSceneListPath[0]);
    }

    void RenderGPU::__loadShaders()
    {
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
        glUniform1i(glGetUniformLocation(thisProgram, "vertexTex"), 3);
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
        glUniform1i(glGetUniformLocation(thisProgram, "vertexTex"), 3);
        glUniform1i(glGetUniformLocation(thisProgram, "normalsTex"), 4);
        glUniform1i(glGetUniformLocation(thisProgram, "uvsTex"), 5);
        glUniform1i(glGetUniformLocation(thisProgram, "materialsTex"), 6);
        glUniform1i(glGetUniformLocation(thisProgram, "transformsTex"), 7);
        glUniform1i(glGetUniformLocation(thisProgram, "lightsTex"), 8);
        glUniform1i(glGetUniformLocation(thisProgram, "textureMapsArrayTex"), 9);
        mRenderPipeline.PathTracerLowResolution->StopUsing();
        Utils::glUtils::checkError("RenderGPU::__loadShaders");
    }

    void RenderGPU::__initGPUDateBuffers()
    {
        std::cerr << Config::LOG_MAGENTA << "Init GPU Buffers" << Config::LOG_RESET;

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        // Create buffer and texture for BVH
        glGenBuffers(1, &mRenderFrameBuffers.BVHBuffer);
        glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHBuffer);
        glBufferData(GL_TEXTURE_BUFFER, sizeof(BVH::BVHFlattor::FlatNode) * mScene->bvhFlattor.flattenedNodes.size(), &mScene->bvhFlattor.flattenedNodes[0], GL_STATIC_DRAW);
        glGenTextures(1, &mRenderFrameBuffers.BVHTex);
        glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.BVHTex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, mRenderFrameBuffers.BVHBuffer);
        // Create buffer and texture for vertex indices
        glGenBuffers(1, &mRenderFrameBuffers.vertexIndicesBuffer);
        glBindBuffer(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexIndicesBuffer);
        glBufferData(GL_TEXTURE_BUFFER, sizeof(int) * mScene->sceneTriIndices.size(), &mScene->sceneTriIndices[0], GL_STATIC_DRAW);
        glGenTextures(1, &mRenderFrameBuffers.vertexIndicesTex);
        glBindTexture(GL_TEXTURE_BUFFER, mRenderFrameBuffers.vertexIndicesTex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, mRenderFrameBuffers.vertexIndicesBuffer);
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
        glGenTextures(1, &mRenderFrameBuffers.transformsTex);
        glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.transformsTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeof(glm::mat4) / sizeof(float) / 4 * mScene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &mScene->transforms[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
        // Create texture for lights
        if (mScene->lights.size() > 0)
        {
            glGenTextures(1, &mRenderFrameBuffers.lightsTex);
            glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.lightsTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, sizeof(Core::Light) / sizeof(float) / 3 * mScene->lights.size(), 1, 0, GL_RGB, GL_FLOAT, &mScene->lights[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // if (!scene->textures.empty())
        // {
        glGenTextures(1, &mRenderFrameBuffers.textureMapsArrayTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mRenderFrameBuffers.textureMapsArrayTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        // }

        // Create texture for scene textures
        // envmap
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
        glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.transformsTex);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, mRenderFrameBuffers.lightsTex);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mRenderFrameBuffers.textureMapsArrayTex);

        std::cerr << " ... " << Config::LOG_GREEN << "Done!" << Config::LOG_RESET << std::endl;
        Utils::glUtils::checkError("RenderGPU::__initGPUDateBuffers");
    }

    void RenderGPU::__initFBOs()
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
        glGenTextures(1, &mRenderFBOs.CPUrenderTexture);
        glBindTexture(GL_TEXTURE_2D, mRenderFBOs.CPUrenderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, lowResSize.x, lowResSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.CPUrenderTexture, 0);

        glGenFramebuffers(1, &mRenderFBOs.accumulationFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulationFBO);
        glGenTextures(1, &mRenderFBOs.accumulationTexture);
        glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulationTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.accumulationTexture, 0);

        glGenFramebuffers(1, &mRenderFBOs.accumulatedFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBOs.accumulatedFBO);
        glGenTextures(1, &mRenderFBOs.accumulatedTexture);
        glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulatedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderFBOs.accumulatedTexture, 0);

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

    void RenderGPU::__captureFrame(unsigned char *buffer)
    {
        // save the current frame to buffer
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mRenderFBOs.outputTexture[1 - currentBuffer]);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        Utils::glUtils::checkError("RenderGPU::__captureFrame");
    }

    void RenderGPU::__captureFrame(float *buffer)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mRenderFBOs.accumulatedTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
        Utils::glUtils::checkError("RenderGPU::__captureFrame");
    }
}

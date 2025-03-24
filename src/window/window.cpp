#include <sstream>
#include <iomanip>
#include <window/window.hpp>
namespace scTracer::Window
{
    Window::Window(bool useGPU) : mWindow(nullptr), mGLManager(std::make_unique<GLFWManager>()), mRenderer(std::make_unique<RenderGPU>(useGPU))
    {
        __autoInit();
        mUseCPU = !useGPU;
        std::cerr << Config::LOG_GREEN << "Every thing is ready!" << Config::LOG_RESET << std::endl;
    }

    Window::~Window()
    {
        if (mWindow)
        {
            glfwDestroyWindow(mWindow);
        }
    }

    void Window::runLoop()
    {
        std::cerr << Config::LOG_BLUE << "Running loop" << Config::LOG_RESET << std::endl;
        __run();
    }

    void Window::__autoInit()
    {
        mWindow = glfwCreateWindow(Config::default_width, Config::default_height, "scTracer", nullptr, nullptr);
        if (!mWindow)
        {
            std::cerr << "Failed to create window" << std::endl;
            glfwTerminate();
            exit(1);
        }
        glfwMakeContextCurrent(mWindow);
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetKeyCallback(mWindow, __keyboardCallback);
        glfwSwapInterval(1);

        gl3wInit();

        __initImGui();

        // z-buffer
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // init renderer
        mRenderer->init();
        mCPURenderer = new CPU::CPURenderer();
        glfwSetWindowSize(mWindow, mRenderer->mScene->settings.image_width, mRenderer->mScene->settings.image_height);
    }

    void Window::__initImGui()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    void Window::__run()
    {
        while (!glfwWindowShouldClose(mWindow))
        {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            __updateImguiWindow();
            glClearColor(0.00f, 0.0f, 0.00f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (!mUseCPU)
                mRenderer->update(); // only for gpu to update gpu data

            if (!mUseCPU)
            {
                mRenderer->render(); // render gpu and show on the screen
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, mRenderer->windowSize.x, mRenderer->windowSize.y);
                mRenderer->show();
            }
            else
            {
                mCPURenderer->render2Canvas(*mRenderer->mScene); // render using cpu
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, mRenderer->windowSize.x, mRenderer->windowSize.y);
                mRenderer->showCPU(mCPURenderer);
            }
            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(mWindow);
        }
    }

    void Window::__updateImguiWindow()
    {
        // update window
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(250, int(250 * (1 + sqrt(5)) / 2)), ImGuiCond_FirstUseEver);
        ImGui::Begin("Control Panel");

        {
            float fps = 1.0f / ImGui::GetIO().DeltaTime;
            ImGui::Text("FPS: %.2f\tspp: %d", fps, mRenderer->numOfSamples);
        }

        {
            std::ostringstream oss, oss1;
            oss << std::fixed << std::setprecision(3) << (mRenderer->timeThisFrame - mRenderer->timeBegin);
            std::string time_string = "cost " + oss.str() + "s";
            oss1 << std::fixed << std::setprecision(1) << 1000 * (mRenderer->timeThisFrame - mRenderer->timeBegin) / mRenderer->numOfSamples;
            if (mRenderer->numOfSamples == mRenderer->mScene->settings.maxSamples)
                time_string += " (" + oss1.str() + "ms per sample)";
            ImGui::Text(time_string.c_str());
            ImGui::Separator();
        }

        if (ImGui::CollapsingHeader("Render Settings"))
        {
            bool isDirty = false;
            bool shaderNeedReload = false;
            bool instancesDirty = false; // materials, transforms, lights
            // max samples (input integer )
            shaderNeedReload |= ImGui::SliderInt("Spp", &mRenderer->mScene->settings.maxSamples, -1, 512);
            shaderNeedReload |= ImGui::SliderInt("Max bounces", &mRenderer->mScene->settings.maxBounceDepth, 1, 32);

            mRenderer->shaderNeedReload |= shaderNeedReload;                         // changes about shaders
            mRenderer->mScene->instancesDirty |= instancesDirty | shaderNeedReload;  // materials, transforms, lights
            mRenderer->mScene->dirty |= isDirty | instancesDirty | shaderNeedReload; // anything changed so we need to flush the canvas
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Capture"))
        {
            ImGui::Text("Save image to file: ");
            static char filename[128] = "output";
            ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
            if (ImGui::Button("Save PNG"))
            {
                std::string fileName = filename;
                if (fileName.find(".png") == std::string::npos)
                    fileName += ".png";
                mRenderer->saveImage(fileName);
                std::cerr << Config::LOG_GREEN << "Saved image to [" << fileName << "]" << Config::LOG_RESET << std::endl;
            }
            if (ImGui::Button("Save EXR"))
            {
                std::string fileName = filename;
                if (fileName.find(".exr") == std::string::npos)
                    fileName += ".exr";
                mRenderer->saveEXR(fileName);
                std::cerr << Config::LOG_GREEN << "Saved image to [" << fileName << "]" << Config::LOG_RESET << std::endl;
            }
        }

        ImGui::Separator();

        // if (ImGui::CollapsingHeader("Camera Info"))
        // {
        //     ImGui::Text("camera info: ");
        //     ImGui::Text("position: %.2f %.2f %.2f", mRenderer->mScene->camera.mPosition.x, mRenderer->mScene->camera.mPosition.y, mRenderer->mScene->camera.mPosition.z);
        //     ImGui::Text("direction: %.2f %.2f %.2f", mRenderer->mScene->camera.mFront.x, mRenderer->mScene->camera.mFront.y, mRenderer->mScene->camera.mFront.z);
        //     // up and right
        //     ImGui::Text("up: %.2f %.2f %.2f", mRenderer->mScene->camera.mUp.x, mRenderer->mScene->camera.mUp.y, mRenderer->mScene->camera.mUp.z);
        //     ImGui::Text("right: %.2f %.2f %.2f", mRenderer->mScene->camera.mRight.x, mRenderer->mScene->camera.mRight.y, mRenderer->mScene->camera.mRight.z);
        // }
        ImGui::End();
    }

    void Window::__keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        // 通过用户指针获取类的实例
        std::cout << "key: " << key << std::endl;
    }
}
#pragma once
#include <fstream>
#include <filesystem>

#include <window/renderer.hpp>
#include <cpu/cpurenderer.hpp>
#include <config.hpp>

namespace scTracer::Window
{

    class Window
    {
    public:
        Window() : mWindow(nullptr), mGLManager(std::make_unique<GLFWManager>()), mRenderer(std::make_unique<RenderGPU>())
        {
            __autoInit();
            std::cerr << Config::LOG_GREEN << "Every thing is ready!" << Config::LOG_RESET << std::endl;
        }

        ~Window()
        {
            if (mWindow)
            {
                glfwDestroyWindow(mWindow);
            }
        }

        void runLoop()
        {
            std::cerr << Config::LOG_BLUE << "Running loop" << Config::LOG_RESET << std::endl;
            __run();
        }
        void useGPU() { mUseCPU = false; }

    private:
        void __autoInit()
        {
            mWindow = glfwCreateWindow(Config::default_width, Config::default_height, "scTracer", nullptr, nullptr);
            if (!mWindow)
            {
                std::cerr << "Failed to create window" << std::endl;
                glfwTerminate();
                exit(1);
            }
            glfwMakeContextCurrent(mWindow);
            glfwSwapInterval(1);
            gl3wInit();

            __initImGui();

            // z-buffer
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // init renderer
            mRenderer->init();
            if (mUseCPU)
                mCPURenderer = new CPU::CPURenderer();
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

        void __run()
        {
            while (!glfwWindowShouldClose(mWindow))
            {
                glfwPollEvents();
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();

                if (!mUseCPU)
                    mRenderer->update(); // only for gpu to update gpu data

                glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                ImGui::NewFrame();
                __updateImguiWindow();
                std::cout << "frame ";
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
                std::cout << "rendered" << std::endl;
                ImGui::Render();

                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(mWindow);
            }
        }

        void __updateImguiWindow()
        {
            // update window
            ImGui::SetNextWindowPos(ImVec2(20, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(20, 40), ImGuiCond_FirstUseEver);

            ImGui::Begin("FPS");
            float fps = 1.0f / ImGui::GetIO().DeltaTime;
            ImGui::Text("FPS: %.1f", fps);
            ImGui::Text("Samples: %d", mRenderer->numOfSamples);
            ImGui::Text("is dirty: %d", mRenderer->mScene->isDirty());
            ImGui::Text("camera info: ");
            ImGui::Text("position: %.2f %.2f %.2f", mRenderer->mScene->camera.mPosition.x, mRenderer->mScene->camera.mPosition.y, mRenderer->mScene->camera.mPosition.z);
            ImGui::Text("direction: %.2f %.2f %.2f", mRenderer->mScene->camera.mFront.x, mRenderer->mScene->camera.mFront.y, mRenderer->mScene->camera.mFront.z);
            // up and right
            ImGui::Text("up: %.2f %.2f %.2f", mRenderer->mScene->camera.mUp.x, mRenderer->mScene->camera.mUp.y, mRenderer->mScene->camera.mUp.z);
            ImGui::Text("right: %.2f %.2f %.2f", mRenderer->mScene->camera.mRight.x, mRenderer->mScene->camera.mRight.y, mRenderer->mScene->camera.mRight.z);
            ImGui::End();
        }

        std::unique_ptr<GLFWManager> mGLManager;
        GLFWwindow *mWindow;
        std::unique_ptr<RenderGPU> mRenderer;

        // for cpu debug
        bool mUseCPU{true};
        CPU::CPURenderer *mCPURenderer{nullptr};
    };
}

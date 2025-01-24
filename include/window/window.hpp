#pragma once
#include <fstream>
#include <filesystem>

#include <window/renderer.hpp>
#include <config.hpp>


namespace scTracer::Window {

    class Window
    {
    public:
        Window() : mWindow(nullptr), mGLManager(std::make_unique<GLFWManager>()), mRenderer(std::make_unique<RenderGPU>()) {
            __autoInit();
        }

        ~Window() {
            if (mWindow) {
                glfwDestroyWindow(mWindow);
            }
        }

        void runLoop() {
            std::cerr << Config::LOG_BLUE << "Running loop" << Config::LOG_RESET << std::endl;
            __run();
        }


    private:

        void __autoInit() {
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
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // init renderer
            mRenderer->init();

        }
        void Window::__initImGui()
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            // Setup Platform/Renderer bindings
            ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
            ImGui_ImplOpenGL3_Init("#version 330");
        }

        void __run() {
            while (!glfwWindowShouldClose(mWindow))
            {
                glfwPollEvents();
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                ImGui::NewFrame();
                __updateWindow();
                ImGui::Render();

                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(mWindow);
            }
        }

        void __updateWindow() {
            // update window
        }

        std::unique_ptr<GLFWManager> mGLManager;
        GLFWwindow* mWindow;

        std::unique_ptr<RenderGPU> mRenderer;

    };
}

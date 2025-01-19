#pragma once
#include <memory>
#include <iostream>
#include <functional>
#include <GL/gl3w.h>
#include <imgui.h>
#include <glfw/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <config.hpp>


namespace scTracer::Window {
    class openGLManager
    {
    public:
        openGLManager() {
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

        ~openGLManager() {
            glfwTerminate();
        }

    private:

    };



    class Window
    {
    public:
        Window() : mWindow(nullptr), mGLManager(std::make_unique<openGLManager>()) {
            __init();
        }

        ~Window() {
            if (mWindow) {
                glfwDestroyWindow(mWindow);
            }
        }

        void runLoop() {
            __run();
        }


    private:
        void __init() {
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

            __setupImGui();

            // z-buffer
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
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
                ImGui::Render();

                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(mWindow);
            }
        }
        void Window::__setupImGui()
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

        std::unique_ptr<openGLManager> mGLManager;
        GLFWwindow* mWindow;
    };
};

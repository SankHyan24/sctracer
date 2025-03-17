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
        Window();
        ~Window();
        void runLoop(); // run the main loop
        inline void useGPU() { mUseCPU = false; }
        inline void useCPU() { mUseCPU = true; }

    private:
        void __autoInit();
        void __initImGui();
        void __run();
        void __updateImguiWindow();
        static void __keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        std::unique_ptr<GLFWManager> mGLManager;
        GLFWwindow *mWindow;
        std::unique_ptr<RenderGPU> mRenderer;
        // for cpu debug
        bool mUseCPU{true};
        CPU::CPURenderer *mCPURenderer{nullptr};
    };
}

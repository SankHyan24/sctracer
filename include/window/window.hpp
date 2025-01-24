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
            __loadSceneLists();
            __loadShaders();
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

        void loadScene(std::string sceneName) {
            __loadScene(sceneName);
        }


    private:
        void __loadSceneLists() {
            // load all scenes
            for (const auto& entry : std::filesystem::directory_iterator(mScenesRootPath))
                if (entry.is_directory()) {
                    // check if has .pbrt file in the folder
                    bool hasPbrt = false;
                    for (const auto& subEntry : std::filesystem::directory_iterator(entry.path()))
                        if (subEntry.is_regular_file() && subEntry.path().extension() == ".pbrt") {
                            hasPbrt = true;
                            break;
                        }
                    if (hasPbrt)
                        mSceneListPath.push_back(entry.path().string().substr(mScenesRootPath.size()));
                }

            std::cout << "Found " << mSceneListPath.size() << " scenes" << std::endl;
            for (const auto& scene : mSceneListPath)
                std::cout << scene << std::endl;

        }

        void __loadScene(std::string sceneName) {
            std::string sceneFullPath = mScenesRootPath + sceneName;
            std::string scenePbrtName;
            for (const auto& entry : std::filesystem::directory_iterator(sceneFullPath))
                if (entry.is_regular_file() && entry.path().extension() == ".pbrt") {
                    scenePbrtName = entry.path().string();
                    break;
                }
            std::cerr << "Loading scene [" << scenePbrtName << "]" << std::endl;
            scTracer::Importer::Pbrt::pbrtParser pbrtScene(scenePbrtName);
        }

        void __loadShaders() {
            scTracer::GPU::Shader vertexShader = scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "vertex.glsl"), GL_VERTEX_SHADER);
            scTracer::GPU::Shader fragDebugger = scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "fragment.glsl"), GL_FRAGMENT_SHADER);
            scTracer::GPU::Program program({ vertexShader, fragDebugger });
        }
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

        std::unique_ptr<GLFWManager> mGLManager;
        GLFWwindow* mWindow;

        std::unique_ptr<RenderGPU> mRenderer;

        // scenes
        std::string mScenesRootPath{ Config::sceneFolder };
        std::vector<std::string> mSceneListPath;
    };
}

#include <iostream>
#include <config.hpp>
#include <window/window.hpp>

#include <core/scene.hpp>

#include <importer/importer.hpp>

#include <gpu/shader.hpp>
int main() {
    std::cout << "Welcome to chuan's Path Tracer!" << std::endl;
    scTracer::Window::Window window;

    window.loadScene("cornell-box");
    window.runLoop();
    std::cout << "Bye!" << std::endl;
    return 0;
}
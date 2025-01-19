#include <iostream>
#include <config.hpp>
#include <window/window.hpp>
#include <core/camera.hpp>
int main() {
    std::cout << "Welcome to chuan's Path Tracer!" << std::endl;
    scTracer::Window::Window window;
    window.runLoop();
    std::cout << "Bye!" << std::endl;
    return 0;
}
#include <iostream>
#include <window/window.hpp>

int main()
{
    std::cout << "Welcome to chuan's Path Tracer!" << std::endl;
    scTracer::Window::Window window(false);
    // window.useGPU();
    window.runLoop();
    std::cout << "Bye!" << std::endl;
    return 0;
}
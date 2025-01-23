#include <iostream>
#include <config.hpp>
#include <window/window.hpp>
#include <core/camera.hpp>

#include <core/material.hpp>
#include <core/mesh.hpp>
#include <core/scene.hpp>

#include <importer/pbrt/parser.hpp>

#include <gpu/shader.hpp>
int main() {
    std::cout << "Welcome to chuan's Path Tracer!" << std::endl;
    scTracer::Window::Window window;

    scTracer::Importer::Pbrt::pbrtParser pbrtScene("assets/cornell-box/scene-v4.pbrt");

    // scTracer::GPU::Shader vertexShader = scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "vertex.glsl"), GL_VERTEX_SHADER);
    // scTracer::GPU::Shader fragDebugger = scTracer::GPU::Shader(scTracer::GPU::shaderRaw::load(scTracer::Config::shaderFolder + "fragment.glsl"), GL_FRAGMENT_SHADER);
    // scTracer::GPU::Program program({ vertexShader, fragDebugger });
    window.runLoop();
    std::cout << "Bye!" << std::endl;
    return 0;
}
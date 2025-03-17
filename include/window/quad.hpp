#pragma once
#include <gpu/program.hpp>
namespace scTracer::Window
{
    class Quad
    {
    public:
        Quad();
        ~Quad();
        void draw(GPU::Program *shader);

    private:
        GLuint vao, vbo;
    };
} // namespace scTracer::Window
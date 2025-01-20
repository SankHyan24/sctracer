#pragma once
#include <vector>
#include <GL/gl3w.h>

#include <config.hpp>
#include <gpu/shader.hpp>

namespace scTracer::GPU
{
    class Program
    { // GL program wrapper, with a Vertex and a Fragment shader
    public:
        Program(const std::vector<Shader> shaders) {
            glID = glCreateProgram();
            // std::cerr << "Linking program\n";
            // attach all shaders
            for (const Shader& shader : shaders)
                glAttachShader(glID, shader.get());
            glLinkProgram(glID);

            // detach all shaders
            for (const Shader& shader : shaders)
                glDetachShader(glID, shader.get());

            GLint success = 0;
            glGetProgramiv(glID, GL_LINK_STATUS, &success);
            if (success == GL_FALSE)
            {
                std::string msg("Error while linking program\n");
                GLint logSize = 0;
                glGetProgramiv(glID, GL_INFO_LOG_LENGTH, &logSize);
                char* info = new char[logSize + 1];
                glGetShaderInfoLog(glID, logSize, NULL, info);
                msg += info;
                delete[] info;
                std::cerr << Config::LOG_RED << "Linking program failed: " << Config::LOG_RESET << msg << std::endl;
                reset();
                exit(1);
            }

        }
        ~Program() {
            reset();
        }
        void Use() {
            glUseProgram(glID);
        }
        void StopUsing() {
            glUseProgram(0);
        }
        GLuint get() const {
            return glID;
        }
    private:
        void reset() {
            if (glID != 0)
            {
                glDeleteProgram(glID);
                glID = 0;
            }
        }
        GLuint glID;
    };
}

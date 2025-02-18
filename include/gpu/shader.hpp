#pragma once
#include <iostream>
#include <string>
#include <fstream>

#include <GL/gl3w.h>

#include <config.hpp>

namespace scTracer::GPU
{
    class shaderRaw
    {
    public:
        shaderRaw() = delete;
        shaderRaw(std::string code, std::string path) : code(code), path(path) {}
        std::string code, path;
        static inline shaderRaw load(std::string path, std::string includeIndentifier = "#include")
        {
            includeIndentifier += ' ';
            static bool isRecursiveCall = false;
            std::string fullSourceCode = "";
            std::ifstream file(path);

            if (!file.is_open())
            {
                std::cerr << "ERROR: could not open the shader at: " << path << "\n"
                          << std::endl;
                return shaderRaw(fullSourceCode, path);
            }

            std::string lineBuffer;
            while (std::getline(file, lineBuffer))
            {
                // Look for the new shader include identifier
                if (lineBuffer.find(includeIndentifier) != lineBuffer.npos)
                {
                    // Remove the include identifier, this will cause the path to remain
                    lineBuffer.erase(0, includeIndentifier.size());

                    // erase "" or <> from the path
                    if (lineBuffer.front() == '\"' || lineBuffer.front() == '<')
                        lineBuffer.erase(0, 1);
                    if (lineBuffer.back() == '\"' || lineBuffer.back() == '>')
                        lineBuffer.pop_back();

                    // The include path is relative to the current shader file path
                    std::string pathOfThisFile;
                    getFilePath(path, pathOfThisFile);
                    lineBuffer.insert(0, pathOfThisFile);

                    // By using recursion, the new include file can be extracted
                    // and inserted at this location in the shader source code
                    isRecursiveCall = true;
                    fullSourceCode += load(lineBuffer).code;

                    // Do not add this line to the shader source code, as the include
                    // path would generate a compilation issue in the final source code
                    continue;
                }

                fullSourceCode += lineBuffer + '\n';
            }

            // Only add the null terminator at the end of the complete file,
            // essentially skipping recursive function calls this way
            if (!isRecursiveCall)
                fullSourceCode += '\0';

            file.close();
            return shaderRaw(fullSourceCode, path);
        }

    private:
        static void getFilePath(const std::string &fullPath, std::string &pathWithoutFileName)
        {
            // Remove the file name and store the path to this folder
            size_t found = fullPath.find_last_of("/\\");
            pathWithoutFileName = fullPath.substr(0, found + 1);
        }
    };
    class Shader
    {
    public:
        GLuint get() const
        {
            return glID;
        }
        Shader(const shaderRaw &raw, GLuint type)
        {
            std::cout << "Compiling shader: " << raw.path << " ......";
            glID = glCreateShader(type);
            const GLchar *src = (const GLchar *)raw.code.c_str();
            glShaderSource(glID, 1, &src, 0);
            glCompileShader(glID);
            GLint success = 0;
            glGetShaderiv(glID, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE)
            {
                std::string msg;
                GLint logSize = 0;
                glGetShaderiv(glID, GL_INFO_LOG_LENGTH, &logSize);
                char *info = new char[logSize + 1];
                glGetShaderInfoLog(glID, logSize, NULL, info);
                msg += raw.path + "\n" + info;
                delete[] info;
                std::cerr << Config::LOG_RED << "Compiling shader[" << Config::LOG_RESET << raw.path
                          << Config::LOG_RED << "] failed: " << Config::LOG_RESET << msg << std::endl;
                reset();
                exit(1);
            }
            std::cout << Config::LOG_GREEN << " Success!" << Config::LOG_RESET << std::endl;
        }
        ~Shader()
        {
            reset();
        }
        void reset()
        {
            if (glID != 0)
            {
                glDeleteShader(glID);
                glID = 0;
            }
        }

    private:
        GLuint glID;
    };
}
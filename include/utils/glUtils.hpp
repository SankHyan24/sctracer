#pragma once

#include <GL/gl3w.h>
#include <string>
#include <chrono>
namespace scTracer::Utils {
    class glUtils {
    public:
        static std::chrono::steady_clock::time_point formalTime;
        static float getNow() {
            auto now = std::chrono::high_resolution_clock::now();
            float res = std::chrono::duration<float, std::milli>(now - formalTime).count();
            formalTime = now;
            return res;
        }
    };

    std::chrono::steady_clock::time_point glUtils::formalTime = std::chrono::high_resolution_clock::now();
}
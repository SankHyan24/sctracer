#pragma once


namespace scTracer::Utils {
    struct mathUtils {
    public:
        static inline float degToRad(float deg) {
            return deg * M_PI / 180.0f;
        }

        static inline float radToDeg(float rad) {
            return rad * 180.0f / M_PI;
        }

        static inline float radians2degrees(float radians) {
            return radians * 180.0f / M_PI;
        }

        static inline float degrees2radians(float degrees) {
            return degrees * M_PI / 180.0f;
        }

        static float M_PI;
    };
}
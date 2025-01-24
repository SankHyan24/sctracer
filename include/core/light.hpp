#pragma once
#include <iostream>
#include <glm/glm.hpp>

namespace scTracer::Core {

    const std::string LightTypeStrings[] = {
        "RectLight",
        "SphereLight",
        "DistantLight"
    };

    enum LightType
    {
        RectLight,
        SphereLight,
        DistantLight
    };

    struct Light
    {
        glm::vec3 position;
        glm::vec3 emission;
        // uv direction
        glm::vec3 u;
        glm::vec3 v;

        float radius{ 0.0f };
        float area{ 0.0f };
        LightType type;

        void printDebugInfo() {
            std::cout << "Light Info" << std::endl;
            std::cout << "position: " << position.x << " " << position.y << " " << position.z << std::endl;
            std::cout << "emission: " << emission.x << " " << emission.y << " " << emission.z << std::endl;
            std::cout << "u: " << u.x << " " << u.y << " " << u.z << std::endl;
            std::cout << "v: " << v.x << " " << v.y << " " << v.z << std::endl;
            std::cout << "radius: " << radius << std::endl;
            std::cout << "area: " << area << std::endl;
            std::cout << "type: " << LightTypeStrings[static_cast<int>(type)] << std::endl;
        }
    };


} // namespace scTracer::Core
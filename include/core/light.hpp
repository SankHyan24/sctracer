#pragma once
#include <iostream>
#include <glm/glm.hpp>

namespace scTracer::Core {

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

        float radius;
        float area;
        float type;
    };


} // namespace scTracer::Core
#pragma once
// #pragma pack(1)
#include <iostream>
#include <glm/glm.hpp>

namespace scTracer::Core
{

    const std::string LightTypeStrings[] = {
        "RectLight",
        "SphereLight",
        "DistantLight"};

    enum LightType
    {
        RectLight,
        SphereLight,
        DistantLight
    };

    class Light
    {
    public:
        glm::vec3 position;
        glm::vec3 emission;
        glm::vec3 u;
        glm::vec3 v;
        float radius{0.0f};
        float area{0.0f};
        float type{0.0f}; // set as float for trans to GPU ezly. cast into int when need

        void printDebugInfo()
        {
            std::cout << "Light Info" << std::endl;
            std::cout << "position: " << position.x << " " << position.y << " " << position.z << std::endl;
            std::cout << "emission: " << emission.x << " " << emission.y << " " << emission.z << std::endl;
            std::cout << "u: " << u.x << " " << u.y << " " << u.z << std::endl;
            std::cout << "v: " << v.x << " " << v.y << " " << v.z << std::endl;
            std::cout << "radius: " << radius << std::endl;
            std::cout << "area: " << area << std::endl;
            std::cout << "type: " << LightTypeStrings[int(type)] << "       type: " << type << std::endl;
        }

        void fromMesh2RectLight(const Mesh *mesh, glm::vec3 emission)
        {
            // only for maya scene
            assert(mesh->vertices.size() == 6 && "RectLight only support quad mesh");
            position = mesh->vertices[0];
            u = mesh->vertices[1] - mesh->vertices[0];
            v = mesh->vertices[2] - mesh->vertices[1];
            radius = 0.5f * glm::length(u) + 0.5f * glm::length(v);
            area = 0.5f * glm::length(glm::cross(u, v));
            type = float(RectLight);
            this->emission = emission;
        }
    };
} // namespace scTracer::Core
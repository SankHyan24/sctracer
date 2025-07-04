#pragma once
#include <bvh/bvh.hpp>

namespace scTracer::Core
{
    class Mesh
    {
    public:
        Mesh()
        {
            bvh = new BVH::BvhStructure();
        };
        ~Mesh()
        {
            delete bvh;
        };

        void printDebugInfo()
        {
            std::cout << "Mesh Debug Info:" << std::endl;
            std::cout << "Vertices: " << vertices.size() << std::endl;
            std::cout << "Normals: " << normals.size() << std::endl;
            std::cout << "UVs: " << uvs.size() << std::endl;
            std::cout << "Indices: " << indices.size() << std::endl;
        }

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<glm::ivec3> indices;

        BVH::BvhStructure *bvh;
        std::string meshName{"Unnamed Mesh"};

        // BVH
        void BuildBVH();
    };
}
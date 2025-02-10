#pragma once
#include <bvh/bvh.hpp>
#include <core/instance.hpp>
#include <core/mesh.hpp>
#include <map>

namespace scTracer::BVH {
    class   BVHFlattor {
    public:
        struct FlatNode {
            glm::vec3 boundsmin;
            glm::vec3  boundsmax;
            glm::vec3 LeftRightLeaf;
        };


        void updateTLAS(const BvhStructure* topLevelBvh, const std::vector<Core::Instance>& instances);
        void flatten(const BvhStructure* topLevelBvh, const std::vector<Core::Mesh*>& meshes, const std::vector<Core::Instance>& instances);

        std::vector<int> bvhRootStartIndices;
        std::vector<FlatNode> flattenedNodes;
        int topLevelIndex{ 0 };
        // context
        int currentTriIndex{ 0 };
        int currentNodeIndex{ 0 };
        int recLvl{ 0 };
        int deepestRecLvl{ 0 };

    private:
        void _flattenBLAS();
        void _flattenTLAS();
        int _flattenBLASNode(const BVH::BvhStructure::Node* node);
        int _flattenTLASNode(const BVH::BvhStructure::Node* node);

        // assets
        const BvhStructure* topLevelBvh;
        std::vector<Core::Mesh*> meshes;
        std::vector<Core::Instance> instances;
    };

    void printFlatNode(const BVHFlattor::FlatNode& node, std::ostream& os);
}
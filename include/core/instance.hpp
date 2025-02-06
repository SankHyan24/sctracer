#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <core/mesh.hpp>
#include <core/material.hpp>
namespace scTracer::Core {
    class Instance {
    public:
        Instance(glm::mat4 transform, int materialIndex, int meshIndex) : mTransform(transform), mMaterialIndex(materialIndex), mMeshIndex(meshIndex) {}
        ~Instance() = default;
        void setTransform(glm::mat4 transform) {
            mTransform = transform;
        }
        glm::mat4 getTransform() {
            return mTransform;
        }
        void printDebugInfo() {
            std::cout << "Instance Debug Info:" << std::endl;
            std::cout << "Material Index: " << mMaterialIndex << std::endl;
            std::cout << "Mesh Index: " << mMeshIndex << std::endl;
        }
        glm::mat4 mTransform;
        int mMaterialIndex{ -1 };
        int mMeshIndex{ -1 };
    };
}
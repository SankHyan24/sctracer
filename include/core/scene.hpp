#pragma once
#include <iostream>
#include <core/camera.hpp>
#include <core/material.hpp>
#include <core/mesh.hpp>
#include <core/instance.hpp>
namespace scTracer::Core {

    struct sceneSettings {
        int image_width;
        int image_height;
        int maxBounceDepth;
        sceneSettings(int image_width, int image_height, int maxBounceDepth) : image_width(image_width), image_height(image_height), maxBounceDepth(maxBounceDepth) {}
        void printDebugInfo() {
            std::cout << "sceneSettings:" << std::endl;
            std::cout << "image_width: " << image_width << std::endl;
            std::cout << "image_height: " << image_height << std::endl;
            std::cout << "maxBounceDepth: " << maxBounceDepth << std::endl;
        }
    };


    class Scene
    {
    public:
        Scene(const Camera& camera, const sceneSettings& settings) : camera(camera), settings(settings) {
        };
        // copy constructor
        Scene(const Scene& scene) : camera(scene.camera), settings(scene.settings) {
            for (auto& material : scene.materials)
                materials.push_back(material);
            for (auto& mesh : scene.meshes)
                meshes.push_back(mesh);
            for (auto& instance : scene.instances)
                instances.push_back(instance);
        }
        ~Scene() {
        }
        void deleteMeshes() {
            for (auto& mesh : meshes)
                delete mesh;
        }
        void printDebugInfo() {
            std::cout << "Scene::printDebugInfo" << std::endl;
            std::cout << "Assets: materials[" << materials.size() << "] meshes[" << meshes.size() << "] instances[" << instances.size() << "]" << std::endl;

            // // camera
            // camera.printDebugInfo();
            // // settings
            // settings.printDebugInfo();
            // // materials
            // for (int i = 0;i < materials.size();i++) {
            //     std::cout << "[" << i << "]" << std::endl;
            //     materials[i].printDebugInfo();
            // }
            // // meshes
            // for (int i = 0;i < meshes.size();i++) {
            //     std::cout << "[" << i << "]" << std::endl;
            //     meshes[i]->printDebugInfo();
            // }
            // // instances
            // for (int i = 0;i < instances.size();i++) {
            //     std::cout << "[" << i << "]" << std::endl;
            //     instances[i].printDebugInfo();
            // }
        }

    private:
        bool dirty{ true };
        bool initialized{ false };
    public:
        // scene settings
        Camera camera;
        sceneSettings settings;

        // assets
        std::vector<Material> materials;
        std::vector<Mesh*> meshes; // pointers to mesh because mesh is a heavy object

        // instances
        std::vector<Instance> instances;
    };

}
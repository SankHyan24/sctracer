#pragma once
#include <iostream>
#include <string>

#include <core/camera.hpp>
#include <core/material.hpp>
#include <core/mesh.hpp>
#include <core/instance.hpp>
#include <core/light.hpp>

#include <bvh/flattenbvh.hpp>
namespace scTracer::Core {

    struct SceneSettings {
        int image_width;
        int image_height;
        int maxBounceDepth;
        SceneSettings(int image_width, int image_height, int maxBounceDepth) : image_width(image_width), image_height(image_height), maxBounceDepth(maxBounceDepth) {}
        void printDebugInfo() {
            std::cout << "SceneSettings:" << std::endl;
            std::cout << "image_width: " << image_width << std::endl;
            std::cout << "image_height: " << image_height << std::endl;
            std::cout << "maxBounceDepth: " << maxBounceDepth << std::endl;
        }
    };

    class Scene
    {
    public:
        Scene(const Camera& camera, const SceneSettings& settings) : camera(camera), settings(settings) {
            sceneBVH = new BVH::BvhStructure();
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
            delete sceneBVH;
            deleteMeshes();
        }
        void processScene() {
            std::cerr << "Building BVH for Meshes ...";
            __createBLAS();
            std::cerr << "Done!" << std::endl;
            std::cerr << "Building BVH for Scene ...";
            __createTLAS();
            std::cerr << "Done!" << std::endl;
            // Flatten BVH
            std::cerr << "Flattening BVH for GPU ...";
            bvhFlattor.flatten(sceneBVH, meshes, instances);
            std::cerr << "Done!" << std::endl;
            // TODO
            // // prepare vertex data
            // int vertexCount = 0;
            // for (auto& mesh : meshes) {
            //     vertexCount += mesh->vertices.size();
            // }
        }

        void deleteMeshes() {
            for (auto& mesh : meshes)
                delete mesh;
        }
        void printDebugInfo() {
            std::cout << "Scene::printDebugInfo" << std::endl;
            std::cout << "Assets: materials[" << materials.size() << "] meshes[" << meshes.size() << "] instances[" << instances.size() << "]" << std::endl;
            // camera
            camera.printDebugInfo();
            // settings
            settings.printDebugInfo();
            // materials
            for (int i = 0;i < materials.size();i++) {
                std::cout << "[" << i << "]" << std::endl;
                materials[i].printDebugInfo();
            }
            // meshes
            for (int i = 0;i < meshes.size();i++) {
                std::cout << "[" << i << "]" << std::endl;
                meshes[i]->printDebugInfo();
            }
            // instances
            for (int i = 0;i < instances.size();i++) {
                std::cout << "[" << i << "]" << std::endl;
                instances[i].printDebugInfo();
            }
            // lights
            for (int i = 0;i < lights.size();i++) {
                std::cout << "[" << i << "]" << std::endl;
                lights[i].printDebugInfo();
            }
        }

    private:
        bool dirty{ true };
        bool initialized{ false };
    public:
        // scene settings
        Camera camera;
        SceneSettings settings;

        // assets
        std::vector<Material> materials;
        std::vector<Mesh*> meshes; // pointers to mesh because mesh is a heavy object
        std::vector<Light> lights;

        // instances
        std::vector<Instance> instances;

    private:
        // for bvh
        BVH::BoundingBox sceneBounds;
        BVH::BvhStructure* sceneBVH;
        BVH::BVHFlattor bvhFlattor;
        void __createBLAS()// create Bottom Level Acceleration Structures(meshes BVH)
        {
#pragma omp parallel for
            for (int i = 0; i < meshes.size(); i++)
                meshes[i]->BuildBVH();
        }
        void __createTLAS()// create Top Level Acceleration Structures(instances BVH)
        {
            std::vector<BVH::BoundingBox> bounds(instances.size());
            for (int i = 0;i < instances.size();i++) {
                BVH::BoundingBox bbox = meshes[instances[i].mMeshIndex]->bvh->getWorldBounds();
                glm::mat4 transform = instances[i].getTransform();

                glm::vec3 minBound = bbox.pmin;
                glm::vec3 maxBound = bbox.pmax;

                // get the new bounds
                glm::vec3 newMinBound = glm::vec3(transform * glm::vec4(minBound, 1.0f));
                glm::vec3 newMaxBound = glm::vec3(transform * glm::vec4(maxBound, 1.0f));

                glm::vec3 right = glm::vec3(transform[0][0], transform[0][1], transform[0][2]);
                glm::vec3 up = glm::vec3(transform[1][0], transform[1][1], transform[1][2]);
                glm::vec3 forward = glm::vec3(transform[2][0], transform[2][1], transform[2][2]);
                glm::vec3 translation = glm::vec3(transform[3][0], transform[3][1], transform[3][2]);

                glm::vec3 xa = right * minBound.x;
                glm::vec3 xb = right * maxBound.x;

                glm::vec3 ya = up * minBound.y;
                glm::vec3 yb = up * maxBound.y;

                glm::vec3 za = forward * minBound.z;
                glm::vec3 zb = forward * maxBound.z;

                minBound = glm::vec3(std::min(xa.x, xb.x) + std::min(ya.x, yb.x) + std::min(za.x, zb.x) + translation.x,
                                     std::min(xa.y, xb.y) + std::min(ya.y, yb.y) + std::min(za.y, zb.y) + translation.y,
                                     std::min(xa.z, xb.z) + std::min(ya.z, yb.z) + std::min(za.z, zb.z) + translation.z);

                maxBound = glm::vec3(std::max(xa.x, xb.x) + std::max(ya.x, yb.x) + std::max(za.x, zb.x) + translation.x,
                                     std::max(xa.y, xb.y) + std::max(ya.y, yb.y) + std::max(za.y, zb.y) + translation.y,
                                     std::max(xa.z, xb.z) + std::max(ya.z, yb.z) + std::max(za.z, zb.z) + translation.z);
                bounds[i].pmin = newMinBound;
                bounds[i].pmax = newMaxBound;
            }
            sceneBVH->build(&bounds[0], bounds.size());
            sceneBounds = sceneBVH->getWorldBounds();
        }
    };

}
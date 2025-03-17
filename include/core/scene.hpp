#pragma once
#include <iostream>
#include <string>

#include <core/camera.hpp>
#include <core/material.hpp>
#include <core/mesh.hpp>
#include <core/instance.hpp>
#include <core/light.hpp>

#include <bvh/flattenbvh.hpp>
namespace scTracer::Core
{

    struct SceneSettings
    {
        int image_width;
        int image_height;
        int maxBounceDepth;
        int maxSamples{64};
        SceneSettings(int image_width, int image_height, int maxBounceDepth, int maxSamples) : image_width(image_width), image_height(image_height), maxBounceDepth(maxBounceDepth), maxSamples(maxSamples) {}
        void printDebugInfo();
    };

    class Scene
    {
    public:
        Scene(const Camera &camera, const SceneSettings &settings);
        Scene::Scene(const Scene &scene);
        ~Scene();

        void processScene();

        void deleteMeshes();
        void printDebugInfo();

        inline bool isDirty() const { return dirty; }
        inline bool isInitialized() const { return initialized; }

        bool dirty{true};
        bool instancesDirty{false};
        bool envMapDirty{false};

        bool initialized{false};

    public:
        // scene settings
        Camera camera;
        SceneSettings settings;
        BVH::BVHFlattor bvhFlattor;

        // assets
        std::vector<MaterialRaw> materials;
        std::vector<Material> materialDatas;
        std::vector<Mesh *> meshes; // pointers to mesh because mesh is a heavy object
        std::vector<Light> lights;

        // meshes data
        std::vector<glm::vec3> sceneVertices;
        std::vector<glm::vec3> sceneNormals;
        std::vector<glm::vec2> sceneMeshUvs;
        std::vector<int> sceneTriIndices;
        // instances data
        std::vector<glm::mat4> transforms;

        // instances
        std::vector<Instance> instances;

    private:
        // for bvh
        BVH::BoundingBox sceneBounds;
        BVH::BvhStructure *sceneBVH;
        void __createBLAS(); // create Bottom Level Acceleration Structures(meshes BVH)
        void __createTLAS(); // create Top Level Acceleration Structures(instances BVH)
    };

}
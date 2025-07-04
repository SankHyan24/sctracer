#include <core/scene.hpp>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
namespace scTracer::Core
{
    void SceneSettings::printDebugInfo()
    {
        std::cout << "SceneSettings:" << std::endl;
        std::cout << "image_width: " << image_width << std::endl;
        std::cout << "image_height: " << image_height << std::endl;
        std::cout << "maxBounceDepth: " << maxBounceDepth << std::endl;
    }

    Scene::Scene(const Camera &camera, const SceneSettings &settings) : camera(camera), settings(settings)
    {
        sceneBVH = new BVH::BvhStructure();
    }

    Scene::Scene(const Scene &scene) : camera(scene.camera), settings(scene.settings)
    {
        for (auto &material : scene.materials)
            materials.push_back(material);
        for (auto &mesh : scene.meshes)
            meshes.push_back(mesh);
        for (auto &instance : scene.instances)
            instances.push_back(instance);
    }

    Scene::~Scene()
    {
        delete sceneBVH;
        deleteMeshes();
    }

    void Scene::processScene()
    {
        std::cerr << "Generating Material Data ...";
        for (int i = 0; i < materials.size(); i++)
        {
            Material material = materials[i].getMaterial();
            materialDatas.push_back(material);
        }
        std::cerr << "Done!" << std::endl;

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

        std::cerr << "Preparing meshes data ...";
        int vertexCount = 0;
        for (int i = 0; i < meshes.size(); i++)
        {
            int numIndex = meshes[i]->bvh->getNumIndices();
            const int *triIndices = &meshes[i]->bvh->mPackedIndices[0];

            for (int j = 0; j < numIndex; j++)
            {
                int index = triIndices[j];
                int v1 = meshes[i]->indices[index].x + vertexCount;
                int v2 = meshes[i]->indices[index].y + vertexCount;
                int v3 = meshes[i]->indices[index].z + vertexCount;
                sceneTriIndices.push_back(v1); // from tri index to global vert index
                sceneTriIndices.push_back(v2);
                sceneTriIndices.push_back(v3);
            }
            sceneVertices.insert(sceneVertices.end(), meshes[i]->vertices.begin(), meshes[i]->vertices.end());
            sceneNormals.insert(sceneNormals.end(), meshes[i]->normals.begin(), meshes[i]->normals.end());
            sceneMeshUvs.insert(sceneMeshUvs.end(), meshes[i]->uvs.begin(), meshes[i]->uvs.end());
            vertexCount += meshes[i]->vertices.size();
        }
        std::cerr << "Done!" << std::endl;

        // prepare instance data(transforms)
        std::cerr << "Preparing instances data ...";
        transforms.resize(instances.size());
        for (int i = 0; i < instances.size(); i++)
            transforms[i] = instances[i].getTransform();
        std::cerr << "Done!" << std::endl;

        // prepare texture data
        std::cerr << "Preparing textures data ...";
        int defaultWidth = Config::default_texture_width;
        int defaultHeight = Config::default_texutre_height;
        int bytesOfOneTexture = defaultWidth * defaultHeight * 4;
        textureMapsData.resize(bytesOfOneTexture * textures.size());
        for (int i = 0; i < textures.size(); i++)
        {
            int texWidth = textures[i]->mWidth;
            int texHeight = textures[i]->mHeight;
            if (texWidth != defaultWidth || texHeight != defaultHeight)
            {
                unsigned char *resizedTex = new unsigned char[bytesOfOneTexture];
                stbir_resize_uint8_linear(&textures[i]->data[0], texWidth, texHeight, 0, resizedTex, defaultWidth, defaultHeight, 0, STBIR_RGBA);
                std::copy(resizedTex, resizedTex + bytesOfOneTexture, &textureMapsData[i * bytesOfOneTexture]);
                delete[] resizedTex;
            }
            else
                std::copy(textures[i]->data.begin(), textures[i]->data.end(), &textureMapsData[i * bytesOfOneTexture]);
        }
        std::cerr << "Done!" << std::endl;

        initialized = true;
    }

    void Scene::deleteMeshes()
    {
        for (auto &mesh : meshes)
            delete mesh;
    }

    void Scene::printDebugInfo()
    {
        std::cout << "Scene::printDebugInfo" << std::endl;
        std::cout << "Assets: materials[" << materials.size() << "] meshes[" << meshes.size() << "] instances[" << instances.size() << "]" << std::endl;
        // camera
        camera.printDebugInfo();
        // settings
        settings.printDebugInfo();
        // materials
        for (int i = 0; i < materials.size(); i++)
        {
            std::cout << "[" << i << "]" << std::endl;
            materials[i].printDebugInfo();
        }
        // meshes
        for (int i = 0; i < meshes.size(); i++)
        {
            std::cout << "[" << i << "]" << std::endl;
            meshes[i]->printDebugInfo();
        }
        // instances
        for (int i = 0; i < instances.size(); i++)
        {
            std::cout << "[" << i << "]" << std::endl;
            instances[i].printDebugInfo();
        }
        // lights
        for (int i = 0; i < lights.size(); i++)
        {
            std::cout << "[" << i << "]" << std::endl;
            // lights[i].printDebugInfo();
        }
    }

    void Scene::__createBLAS()
    {
        for (int i = 0; i < meshes.size(); i++)
            meshes[i]->BuildBVH();
    }

    void Scene::__createTLAS()
    {
        std::vector<BVH::BoundingBox> bounds(instances.size());
        for (int i = 0; i < instances.size(); i++)
        {
            if (!instances[i].mActived)
                continue;
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
}
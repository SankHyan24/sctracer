#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>

#include <core/scene.hpp>

namespace scTracer::Importer::Pbrt
{

    class pbrtSceneBlock
    {
    public:
        const std::vector<std::string> blockTypeStrings{
            "Camera",
            "Film",
            "Integrator",
            "Transform",
            "WorldBegin",
            "MakeNamedMaterial",
            "NamedMaterial",
            "Shape",
            "AttributeBegin",
            "AttributeEnd",
            "Unsupported"};
        enum class BlockType
        {
            Camera,
            Film,
            Integrator,
            Transform,
            WorldBegin,
            MakeNamedMaterial,
            NamedMaterial,
            Shape,
            AttributeBegin,
            AttributeEnd,
            Unsupported
        };
        BlockType mBType;
        pbrtSceneBlock(const std::vector<std::string> &content) : mContent(content)
        {
            mBType = getBlockType(mContent[0]);
            // std::cout << "Block Type: " << blockTypeStrings[static_cast<int>(mBType)] << std::endl;
        }
        ~pbrtSceneBlock() = default;

    private:
        static BlockType getBlockType(std::string firstLine)
        {
            if (firstLine.find("Camera") != std::string::npos)
            {
                return BlockType::Camera;
            }
            if (firstLine.find("Film") != std::string::npos)
            {
                return BlockType::Film;
            }
            if (firstLine.find("Integrator") != std::string::npos)
            {
                return BlockType::Integrator;
            }
            if (firstLine.find("Transform") != std::string::npos)
            {
                return BlockType::Transform;
            }
            if (firstLine.find("WorldBegin") != std::string::npos)
            {
                return BlockType::WorldBegin;
            }
            if (firstLine.find("MakeNamedMaterial") != std::string::npos)
            {
                return BlockType::MakeNamedMaterial;
            }
            if (firstLine.find("NamedMaterial") != std::string::npos)
            {
                return BlockType::NamedMaterial;
            }
            if (firstLine.find("Shape") != std::string::npos)
            {
                return BlockType::Shape;
            }
            if (firstLine.find("AttributeBegin") != std::string::npos)
            {
                return BlockType::AttributeBegin;
            }
            if (firstLine.find("AttributeEnd") != std::string::npos)
            {
                return BlockType::AttributeEnd;
            }
            return BlockType::Unsupported;
        }
        std::vector<std::string> mContent;

    public:
        glm::mat4 getTransform()
        {
            assert(mBType == BlockType::Transform);
            std::string transformString = mContent[0];
            transformString = transformString.substr(transformString.find("[") + 1, transformString.find("]") - transformString.find("[") - 1);
            std::vector<float> transformValues;
            std::string value;
            for (int i = 0; i < transformString.size(); i++)
                if (transformString[i] == ' ' && value.size() > 0)
                {
                    transformValues.push_back(std::stof(value));
                    value.clear();
                }
                else
                    value += transformString[i];
            glm::mat4 transform;
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    transform[i][j] = transformValues[i * 4 + j];
            return transform;
        }

        float getCameraFOV()
        {
            assert(mBType == BlockType::Camera);
            std::string cameraString = mContent[1];
            assert(cameraString.find("fov") != std::string::npos);
            cameraString = cameraString.substr(cameraString.find("[") + 1, cameraString.find("]") - cameraString.find("[") - 1);
            return std::stof(cameraString);
        }

        int getMaxBounceDepth()
        {
            assert(mBType == BlockType::Integrator);
            std::string integratorString = mContent[1];
            integratorString = integratorString.substr(integratorString.find("[") + 1, integratorString.find("]") - integratorString.find("[") - 1);
            return std::stoi(integratorString);
        }

        std::vector<int> getResolution(std::string &outputFileName)
        {
            assert(mBType == BlockType::Film);
            std::vector<int> resolution;
            for (int i = 1; i < mContent.size(); i++)
            {
                if (mContent[i].find("xresolution") != std::string::npos)
                {
                    std::string xres = mContent[i];
                    xres = xres.substr(xres.find("[") + 1, xres.find("]") - xres.find("[") - 1);
                    resolution.push_back(std::stoi(xres));
                }
                if (mContent[i].find("yresolution") != std::string::npos)
                {
                    std::string yres = mContent[i];
                    yres = yres.substr(yres.find("[") + 1, yres.find("]") - yres.find("[") - 1);
                    resolution.push_back(std::stoi(yres));
                }
                if (mContent[i].find("filename") != std::string::npos)
                {
                    outputFileName = mContent[i];
                    outputFileName = outputFileName.substr(outputFileName.find("[") + 1, outputFileName.find("]") - outputFileName.find("[") - 1);
                }
            }
            outputFileName = outputFileName.substr(1, outputFileName.size() - 2);
            return resolution;
        }

        Core::MaterialRaw getMaterial()
        {
            assert(mBType == BlockType::MakeNamedMaterial);
            std::string materialName = mContent[0];
            materialName = materialName.substr(materialName.find("\"") + 1, materialName.find("\"", materialName.find("\"") + 1) - materialName.find("\"") - 1);
            std::string materialType;
            std::vector<float> reflectance;
            for (int i = 1; i < mContent.size(); i++)
            {
                if (mContent[i].find("string type") != std::string::npos)
                {
                    materialType = mContent[i];
                    materialType = materialType.substr(materialType.find("[") + 1, materialType.find("]") - materialType.find("[") - 1);
                }
                if (mContent[i].find("rgb reflectance") != std::string::npos)
                {
                    std::string reflectanceString = mContent[i];
                    reflectanceString = reflectanceString.substr(reflectanceString.find("[") + 1, reflectanceString.find("]") - reflectanceString.find("[") - 1);
                    std::string value;
                    for (int i = 0; i < reflectanceString.size(); i++)
                        if (reflectanceString[i] == ' ' && value.size() > 0)
                        {
                            reflectance.push_back(std::stof(value));
                            value.clear();
                        }
                        else
                            value += reflectanceString[i];
                }
            }
            Core::MaterialRaw material;
            material.name = materialName;
            material.basecolor = glm::vec3(reflectance[0], reflectance[1], reflectance[2]);
            material.mType = Core::getMaterialType(materialType);
            material.typePreprocess();
            return material;
        }

        std::string getMaterialName()
        {
            assert(mBType == BlockType::NamedMaterial);
            std::string materialName = mContent[0];
            materialName = materialName.substr(materialName.find("\"") + 1, materialName.find("\"", materialName.find("\"") + 1) - materialName.find("\"") - 1);
            return materialName;
        }

        Core::Mesh *getMeshFromFile()
        {
            assert(mBType == BlockType::Shape);
            Core::Mesh *mesh = new Core::Mesh();
            std::string uvString, normalString, positionString, indexString;
            std::string *currentString = nullptr;

            // split the content into position, uv, normal, and index strings
            for (int i = 1; i < mContent.size(); i++)
            {
                if (mContent[i].find("point3 P") != std::string::npos)
                {
                    currentString = &positionString;
                    *currentString += mContent[i];
                    continue;
                }
                if (mContent[i].find("point2 uv") != std::string::npos)
                {
                    currentString = &uvString;
                    *currentString += mContent[i];
                    continue;
                }
                if (mContent[i].find("normal N") != std::string::npos)
                {
                    currentString = &normalString;
                    *currentString += mContent[i];
                    continue;
                }
                if (mContent[i].find("integer indices") != std::string::npos)
                {
                    currentString = &indexString;
                    *currentString += mContent[i];
                    continue;
                }
                *currentString += " " + mContent[i];
            }

            // get the values from the strings
            positionString = positionString.substr(positionString.find("[") + 1, positionString.find("]") - positionString.find("[") - 1);
            uvString = uvString.substr(uvString.find("[") + 1, uvString.find("]") - uvString.find("[") - 1);
            normalString = normalString.substr(normalString.find("[") + 1, normalString.find("]") - normalString.find("[") - 1);
            indexString = indexString.substr(indexString.find("[") + 1, indexString.find("]") - indexString.find("[") - 1);

            // remove extra spaces
            for (int i = 0; i < positionString.size(); i++)
                if (positionString[i] == ' ' && positionString[i + 1] == ' ')
                {
                    positionString.erase(i, 1);
                    i--;
                }
            for (int i = 0; i < uvString.size(); i++)
                if (uvString[i] == ' ' && uvString[i + 1] == ' ')
                {
                    uvString.erase(i, 1);
                    i--;
                }
            for (int i = 0; i < normalString.size(); i++)
                if (normalString[i] == ' ' && normalString[i + 1] == ' ')
                {
                    normalString.erase(i, 1);
                    i--;
                }
            for (int i = 0; i < indexString.size(); i++)
                if (indexString[i] == ' ' && indexString[i + 1] == ' ')
                {
                    indexString.erase(i, 1);
                    i--;
                }

            // get values from strings to vectors
            std::vector<float> positionValues;
            std::vector<float> uvValues;
            std::vector<float> normalValues;
            std::vector<int> indexValues;
            std::string value;

            for (int i = 0; i < positionString.size(); i++)
                if (positionString[i] == ' ' && value.size() > 0)
                {
                    positionValues.push_back(std::stof(value));
                    value.clear();
                }
                else
                    value += positionString[i];
            value.clear();
            for (int i = 0; i < uvString.size(); i++)
                if (uvString[i] == ' ' && value.size() > 0)
                {
                    uvValues.push_back(std::stof(value));
                    value.clear();
                }
                else
                    value += uvString[i];
            value.clear();
            for (int i = 0; i < normalString.size(); i++)
                if (normalString[i] == ' ' && value.size() > 0)
                {
                    normalValues.push_back(std::stof(value));
                    value.clear();
                }
                else
                    value += normalString[i];
            value.clear();
            for (int i = 0; i < indexString.size(); i++)
                if (indexString[i] == ' ' && value.size() > 0)
                {
                    indexValues.push_back(std::stoi(value));
                    value.clear();
                }
                else
                    value += indexString[i];

            for (int i = 0; i < positionValues.size(); i += 3)
                mesh->vertices.push_back(glm::vec3(positionValues[i], positionValues[i + 1], positionValues[i + 2]));
            for (int i = 0; i < uvValues.size(); i += 2)
                mesh->uvs.push_back(glm::vec2(uvValues[i], uvValues[i + 1]));
            for (int i = 0; i < normalValues.size(); i += 3)
                mesh->normals.push_back(glm::vec3(normalValues[i], normalValues[i + 1], normalValues[i + 2]));
            for (int i = 0; i < indexValues.size(); i += 3)
                mesh->indices.push_back(glm::ivec3(indexValues[i], indexValues[i + 1], indexValues[i + 2]));

            return mesh;
        }

        Core::Mesh *getMeshFromOtherFile() {}

        Core::Light getLight()
        {
            assert(mBType == BlockType::AttributeBegin);
            Core::Light light;
            std::string lightMatTypeline; // @sc: only for diffuse now
            std::string materialName;
            lightMatTypeline = mContent[1];
            materialName = lightMatTypeline.substr(lightMatTypeline.find("\"") + 1, lightMatTypeline.find("\"", lightMatTypeline.find("\"") + 1) - lightMatTypeline.find("\"") - 1);
            lightMatTypeline = lightMatTypeline.substr(0, lightMatTypeline.find("\"") - 1);

            std::vector<std::string> lightDetails;
            bool index_lightShapeBegin{false};
            for (int i = 2; i < mContent.size(); i++)
            {
                if (mContent[i].find("rgb L") != std::string::npos)
                {
                    std::string emissionString = mContent[i];
                    emissionString = emissionString.substr(emissionString.find("[") + 1, emissionString.find("]") - emissionString.find("[") - 1);
                    std::vector<float> emissionValues;
                    std::string value;
                    for (int i = 0; i < emissionString.size(); i++)
                        if (emissionString[i] == ' ' && value.size() > 0)
                        {
                            emissionValues.push_back(std::stof(value));
                            value.clear();
                        }
                        else
                            value += emissionString[i];
                    light.emission = glm::vec3(emissionValues[0], emissionValues[1], emissionValues[2]);
                }
                if (mContent[i].find("NamedMaterial") != std::string::npos)
                {
                    index_lightShapeBegin = true;
                }
                if (index_lightShapeBegin)
                    lightDetails.push_back(mContent[i]);
            }

            int lightDetailsIndex{0};
            std::string lightTransformString;
            glm::mat4 lightTransform;
            bool have_lightTransform{false};

            for (int i = 0; i < lightDetails.size(); i++)
                if (lightDetails[i].find("Transform") != std::string::npos)
                {
                    lightTransformString = lightDetails[i];
                    lightDetailsIndex = i;
                    have_lightTransform = true;
                    break;
                }

            if (have_lightTransform)
            {
                lightTransformString = lightTransformString.substr(lightTransformString.find("[") + 1, lightTransformString.find("]") - lightTransformString.find("[") - 1);
                std::vector<float> transformValues;
                std::string value;
                for (int i = 0; i < lightTransformString.size(); i++)
                    if (lightTransformString[i] == ' ' && value.size() > 0)
                    {
                        transformValues.push_back(std::stof(value));
                        value.clear();
                    }
                    else
                        value += lightTransformString[i];
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 4; j++)
                        lightTransform[i][j] = transformValues[i * 4 + j];
            }

            std::string lightShapeTypeString;
            for (; lightDetailsIndex < lightDetails.size(); lightDetailsIndex++)
                if (lightDetails[lightDetailsIndex].find("Shape") != std::string::npos)
                {
                    lightShapeTypeString = lightDetails[lightDetailsIndex];
                    break;
                }
            // trianglemesh sphere direction(not implemented)
            if (lightShapeTypeString.find("trianglemesh") != std::string::npos)
            {
                light.type = Core::LightType::RectLight;
                light.type = float(Core::LightType::RectLight);
                std::vector<glm::vec3> positions;
                for (; lightDetailsIndex < lightDetails.size(); lightDetailsIndex++)
                    if (lightDetails[lightDetailsIndex].find("point3 P") != std::string::npos)
                    {
                        std::string positionString = lightDetails[lightDetailsIndex];
                        positionString = positionString.substr(positionString.find("[") + 1, positionString.find("]") - positionString.find("[") - 1);
                        std::vector<float> positionValues;
                        std::string value;
                        for (int i = 0; i < positionString.size(); i++)
                            if (positionString[i] == ' ' && value.size() > 0)
                            {
                                positionValues.push_back(std::stof(value));
                                value.clear();
                            }
                            else
                                value += positionString[i];
                        // get the positions
                        for (int i = 0; i < positionValues.size(); i += 3)
                            positions.push_back(glm::vec3(positionValues[i], positionValues[i + 1], positionValues[i + 2]));
                        break;
                    }
                assert(positions.size() == 4 && "Only support rectangle light now");
                light.position = positions[0];
                light.u = positions[1] - positions[0];
                light.v = positions[3] - positions[0];
                light.area = glm::length(glm::cross(light.u, light.v));
            }
            else if (lightShapeTypeString.find("sphere") != std::string::npos)
            {
                light.type = Core::LightType::SphereLight;
                light.type = float(Core::LightType::SphereLight);
                for (; lightDetailsIndex < lightDetails.size(); lightDetailsIndex++)
                    if (lightDetails[lightDetailsIndex].find("float radius") != std::string::npos)
                    {
                        std::string radiusString = lightDetails[lightDetailsIndex];
                        radiusString = radiusString.substr(radiusString.find("[") + 1, radiusString.find("]") - radiusString.find("[") - 1);
                        light.position = glm::vec3(lightTransform[3][0], lightTransform[3][1], lightTransform[3][2]);
                        light.radius = std::stof(radiusString);
                        light.area = 4.0f * glm::pi<float>() * light.radius * light.radius;
                        break;
                    }
            }
            else
                assert(false && "Unsupported light shape type");
            return light;
        }

    private:
    };

    class pbrtSceneFile
    { // from file string to code blocks
    public:
        pbrtSceneFile(const std::string &path) : mFilePath(path)
        {
            loadFile();
        }
        ~pbrtSceneFile() = default;

        std::vector<pbrtSceneBlock> mBlocks;

    private:
        void loadFile()
        {
            std::ifstream file(mFilePath);
            if (!file.is_open())
            {
                std::cerr << "Error: Could not open file " << mFilePath << std::endl;
                return;
            }
            std::string line;
            while (std::getline(file, line))
            {
                mContentLine.push_back(line);
            }
            std::vector<std::string> thisBlock;
            for (int i = 0; i < mContentLine.size(); i++)
            {
                if (mContentLine[i].size() == 0)
                    continue;
                if (mContentLine[i][0] != ' ')
                {
                    if (thisBlock.size() > 0)
                    {
                        mBlocks.push_back(pbrtSceneBlock(thisBlock));
                        thisBlock.clear();
                    }
                }
                thisBlock.push_back(mContentLine[i]);
            }
        }
        std::string mFilePath;
        std::vector<std::string> mContentLine;
    };

    class pbrtParser
    {
    public:
        pbrtParser() = default;
        pbrtParser(const std::string &path)
        {
            parse(path);
        };
        ~pbrtParser() = default;
        static Core::Scene *parse(const std::string &path)
        {
            std::cout << "start parsing file: " << path << "......";
            auto &blocks = pbrtSceneFile(path).mBlocks;
            bool world_begin{false};
            // SCENE SETTINGS
            glm::mat4 camera_transform;
            float camera_fov{19.5};
            int max_bounce_depth{1024};
            int resolution_x{800};
            int resolution_y{600};
            std::string output_file_name;

            for (auto &block : blocks)
            {
                if (world_begin)
                    break;
                switch (block.mBType)
                {
                case pbrtSceneBlock::BlockType::Camera:
                    camera_fov = block.getCameraFOV();
                    break;
                case pbrtSceneBlock::BlockType::Transform:
                    camera_transform = block.getTransform();
                    break;
                case pbrtSceneBlock::BlockType::Integrator:
                    max_bounce_depth = block.getMaxBounceDepth();
                    break;
                case pbrtSceneBlock::BlockType::Film:
                {
                    std::vector<int> resolution = block.getResolution(output_file_name);
                    resolution_x = resolution[0];
                    resolution_y = resolution[1];
                    break;
                }
                case pbrtSceneBlock::BlockType::WorldBegin:
                    world_begin = true;
                    break;
                case pbrtSceneBlock::BlockType::Unsupported:
                    // assert(false && "Unsupported block type");
                default:
                    break;
                }
            }
            // world begin
            std::vector<Core::MaterialRaw> materials;
            std::vector<Core::Mesh *> meshes;
            std::vector<Core::Instance> instances;
            std::vector<Core::Light> lights;
            int currentMaterialIndex{-1};
            bool attribute_begin{false};
            for (auto &block : blocks)
            {
                switch (block.mBType)
                {
                case pbrtSceneBlock::BlockType::MakeNamedMaterial:
                {
                    materials.push_back(block.getMaterial());
                    break;
                }
                case pbrtSceneBlock::BlockType::NamedMaterial:
                {
                    currentMaterialIndex = -1;
                    std::string name = block.getMaterialName();
                    for (int i = 0; i < materials.size(); i++)
                        if (materials[i].name == name)
                        {
                            currentMaterialIndex = i;
                            break;
                        }
                    assert(currentMaterialIndex != -1 && "MaterialRaw not found");
                    break;
                }
                case pbrtSceneBlock::BlockType::Shape:
                {
                    Core::Mesh *mesh = block.getMeshFromFile();
                    meshes.push_back(mesh);
                    instances.push_back(Core::Instance(glm::mat4(1.0f), currentMaterialIndex, meshes.size() - 1));
                    break;
                }
                case pbrtSceneBlock::BlockType::AttributeBegin:
                {
                    attribute_begin = true;
                    lights.push_back(block.getLight());
                    break;
                }
                case pbrtSceneBlock::BlockType::AttributeEnd:
                {
                    attribute_begin = false;
                    break;
                }
                default:
                    break;
                }
            }
            assert(world_begin && "WorldBegin not found");
            assert(attribute_begin && "AttributeEnd not found");
            auto scene = new Core::Scene(Core::Camera(camera_transform, camera_fov), Core::SceneSettings(resolution_x, resolution_y, max_bounce_depth));
            scene->materials = materials;
            int meshCnter{0};
            for (auto &mesh : meshes)
            {
                mesh->meshName = "inline_mesh_[" + std::to_string(meshCnter++) + "]";
                scene->meshes.push_back(mesh);
            }
            for (auto &instance : instances)
            {
                scene->instances.push_back(instance);
            }
            for (auto &light : lights)
                scene->lights.push_back(light);
            std::cerr << Config::LOG_GREEN << "Done!" << Config::LOG_RESET << std::endl;
            // scene->printDebugInfo();
            return scene;
        }
    };
}
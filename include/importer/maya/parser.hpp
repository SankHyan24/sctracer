#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <tuple>
#include <core/scene.hpp>

#include <tinyxml2.h>
#include <tiny_obj_loader.h>

namespace scTracer::Importer::Maya
{
    // this parser only works for the ZJU CG course's scene files
    class mayaObjFile
    {
    public:
        std::string objPath;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;  // 材质数据
        std::vector<Core::Mesh *> meshes;            // mesh数据
        std::vector<Core::MaterialRaw> materialsRaw; // 材质数据
        std::vector<Core::Instance> instances;       // 实例数据
        mayaObjFile(std::string objPath) : objPath(objPath)
        {
            __parse();
        }

        void __parse()
        {
            tinyobj::ObjReaderConfig reader_config;
            tinyobj::attrib_t attrib;
            reader_config.triangulate = true;
            reader_config.vertex_color = false;
            int lastIndex = objPath.find_last_of("/");
            int lastIndex1 = objPath.find_last_of("\\");
            if (lastIndex1 > lastIndex)
                lastIndex = lastIndex1;
            reader_config.mtl_search_path = objPath.substr(0, lastIndex);

            tinyobj::ObjReader reader;
            std::string warn; // 警告信息
            std::string err;  // 错误信息
            bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str(), reader_config.mtl_search_path.c_str(), true);
            if (!success)
            {
                std::cerr << "Error: " << err << std::endl;
                exit(0);
            }
            const std::vector<float> &vertices = attrib.vertices;
            const std::vector<float> &normals = attrib.normals;
            const std::vector<float> &texcoords = attrib.texcoords;
            const std::vector<tinyobj::shape_t> &shapes = this->shapes;
            const std::vector<tinyobj::material_t> &materials = this->materials;

            // add materials
            for (int i = 0; i < materials.size(); i++)
            {
                Core::MaterialRaw mat;
                mat.name = materials[i].name;
                mat.mSystem = Core::MaterialRaw::MatSystem::PHONG;
                mat.Kd = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
                mat.Ks = glm::vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
                mat.Tr = glm::vec3(materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
                mat.Ns = materials[i].shininess;
                mat.Ni = materials[i].ior;
                mat.baseColorTexId = -1;
                if (materials[i].diffuse_texname != "")
                {
                    mat.baseColorTexId = i;
                    mat.baseColorTexName = materials[i].diffuse_texname;
                }
                materialsRaw.push_back(mat);
            }

            // add meshes
            for (int i = 0; i < shapes.size(); i++)
            {
                auto &shape = shapes[i];
                auto &mesh = shape.mesh;
                Core::Mesh *curMesh = new Core::Mesh();
                curMesh->meshName = shape.name;
                for (size_t j = 0; j < mesh.num_face_vertices.size(); j++)
                {
                    int fnum = mesh.num_face_vertices[j];
                    assert(fnum == 3 && "only support triangle mesh");
                    for (int k = 0; k < fnum; k++)
                    {
                        tinyobj::index_t index = mesh.indices[j * fnum + k];
                        glm::vec3 v = glm::vec3(vertices[3 * index.vertex_index + 0], vertices[3 * index.vertex_index + 1], vertices[3 * index.vertex_index + 2]);
                        glm::vec3 n = glm::vec3(normals[3 * index.normal_index + 0], normals[3 * index.normal_index + 1], normals[3 * index.normal_index + 2]);
                        glm::vec2 uv = glm::vec2(texcoords[2 * index.texcoord_index + 0], texcoords[2 * index.texcoord_index + 1]);
                        curMesh->vertices.push_back(v);
                        curMesh->normals.push_back(n);
                        curMesh->uvs.push_back(uv);
                    }
                    // add face indices (local vert index)
                    curMesh->indices.push_back(glm::ivec3(3 * j + 0, 3 * j + 1, 3 * j + 2));
                }
                // add instance
                Core::Instance instance(glm::mat4(1.0f), mesh.material_ids[0], i);
                meshes.push_back(curMesh);
                instances.push_back(instance);
            }
        }
    };
    class mayaMtlFile
    {
    public:
        std::string mtlPath;
        std::vector<Core::MaterialRaw> materials;
        mayaMtlFile(std::string mtlPath) : mtlPath(mtlPath)
        {
            __parse();
        }
        void __parse()
        {
            std::string contents;
            std::vector<std::string> lines;
            std::ifstream file(mtlPath);
            if (!file.is_open())
            {
                std::cerr << "Error: Could not open file " << mtlPath << std::endl;
                exit(0);
            }
            std::string line;
            while (std::getline(file, line))
            {
                lines.push_back(line);
            }
            int scanIndex = 0;
            while (scanIndex < lines.size())
            {
                if (lines[scanIndex].size() == 0) // empty line
                {
                    scanIndex++;
                    continue;
                }
                std::string line = lines[scanIndex];
                if (line.find("newmtl") != std::string::npos) // head line -> find newmtl
                {
                    materials.push_back(Core::MaterialRaw());
                    materials.back().name = line.substr(line.find("newmtl") + 7);
                    materials.back().mSystem = Core::MaterialRaw::MatSystem::PHONG;
                    scanIndex++;
                    continue;
                }
                assert(materials.size() > 0 && "material size error");
                if (line.find("Kd") != std::string::npos && line.find("map_Kd") == std::string::npos)
                {
                    std::string value = line.substr(line.find("Kd") + 3);
                    std::vector<float> values;
                    std::stringstream ss(value);
                    float temp;
                    while (ss >> temp)
                    {
                        values.push_back(temp);
                        if (ss.peek() == ',')
                            ss.ignore();
                    }
                    std::cout << line << std::endl;
                    std::cout << values.size() << std::endl;
                    assert(values.size() == 3 && "Kd value error");
                    materials.back().Kd = glm::vec3(values[0], values[1], values[2]);

                    scanIndex++;
                    continue;
                }
                if (line.find("map_Kd") != std::string::npos)
                {
                    // TODO: texture
                    scanIndex++;
                }
                if (line.find("Ks") != std::string::npos)
                {
                    std::string value = line.substr(line.find("Ks") + 3);
                    std::vector<std::string> values;
                    std::string temp;
                    for (int i = 0; i < value.size(); i++)
                    {
                        if (value[i] == ' ')
                        {
                            values.push_back(temp);
                            temp.clear();
                        }
                        else
                            temp += value[i];
                    }
                    values.push_back(temp);
                    assert(values.size() == 3 && "Ks value error");
                    materials.back().Ks = glm::vec3(std::stof(values[0]), std::stof(values[1]), std::stof(values[2]));
                    scanIndex++;
                    continue;
                }
                if (line.find("Tr") != std::string::npos)
                {
                    std::string value = line.substr(line.find("Tr") + 3);
                    std::vector<std::string> values;
                    std::string temp;
                    for (int i = 0; i < value.size(); i++)
                    {
                        if (value[i] == ' ')
                        {
                            values.push_back(temp);
                            temp.clear();
                        }
                        else
                            temp += value[i];
                    }
                    values.push_back(temp);
                    assert(values.size() == 3 && "Tr value error");
                    materials.back().Tr = glm::vec3(std::stof(values[0]), std::stof(values[1]), std::stof(values[2]));
                    scanIndex++;
                    continue;
                }
                if (line.find("Ns") != std::string::npos)
                {
                    std::string value = line.substr(line.find("Ns") + 3);
                    materials.back().Ns = std::stof(value);
                    scanIndex++;
                    continue;
                }
                if (line.find("Ni") != std::string::npos)
                {
                    std::string value = line.substr(line.find("Ni") + 3);
                    materials.back().Ni = std::stof(value);
                    scanIndex++;
                    continue;
                }
                scanIndex++;
            }
        }
    };
    class mayaXmlFile
    {
    public:
        std::string xmlPath;
        Core::Camera camera{glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), 90.0f};
        Core::SceneSettings settings{800, 600, 5, 64};
        std::vector<std::string> lightsNames;
        std::vector<glm::vec3> lightsEmissions;
        mayaXmlFile(std::string xmlPath) : xmlPath(xmlPath)
        {
            __parse();
        }
        void __parse()
        {
            tinyxml2::XMLDocument doc;
            if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS)
            {
                std::cerr << "cannot load XML file " << doc.ErrorStr() << std::endl;
                exit(0);
            }
            tinyxml2::XMLElement *camera_ = doc.FirstChildElement();
            if (!camera_)
                exit(0);
            assert(std::string(camera_->Name()) == "camera");
            glm::vec3 pos;
            auto pos_ = camera_->FirstChildElement();
            auto pos_x = pos_->FirstAttribute();
            auto pos_y = pos_x->Next();
            auto pos_z = pos_y->Next();
            pos.x = pos_x->FloatValue();
            pos.y = pos_y->FloatValue();
            pos.z = pos_z->FloatValue();
            glm::vec3 lookAt;
            auto lkat_ = pos_->NextSiblingElement();
            auto lkat_x = lkat_->FirstAttribute();
            auto lkat_y = lkat_x->Next();
            auto lkat_z = lkat_y->Next();
            lookAt.x = lkat_x->FloatValue();
            lookAt.y = lkat_y->FloatValue();
            lookAt.z = lkat_z->FloatValue();
            glm::vec3 up;
            auto up_ = lkat_->NextSiblingElement();
            auto up_x = up_->FirstAttribute();
            auto up_y = up_x->Next();
            auto up_z = up_y->Next();
            up.x = up_x->FloatValue();
            up.y = up_y->FloatValue();
            up.z = up_z->FloatValue();
            int res_x, res_y;
            float fovY;
            auto projType_ = camera_->FirstAttribute();
            auto res_x_ = projType_->Next();
            auto res_y_ = res_x_->Next();
            auto fovY_ = res_y_->Next();
            res_x = res_x_->IntValue();
            res_y = res_y_->IntValue();
            fovY = fovY_->FloatValue();
            camera = Core::Camera(pos, lookAt, fovY, up);
            settings = Core::SceneSettings(res_x, res_y);
            tinyxml2::XMLElement *firstlight_ = camera_->NextSiblingElement();
            for (auto light_ = firstlight_; light_ != nullptr; light_ = light_->NextSiblingElement())
            {
                assert(std::string(light_->Name()) == "light");
                auto name_ = light_->FirstAttribute();
                std::string emission_ = name_->Next()->Value(); // radiance="300,300,300"
                glm::vec3 emission;
                // split the string
                std::vector<std::string> value;
                std::string temp;
                for (int i = 0; i < emission_.size(); i++)
                {
                    if (emission_[i] == ',')
                    {
                        value.push_back(temp);
                        temp.clear();
                    }
                    else
                        temp += emission_[i];
                }
                value.push_back(temp);
                assert(value.size() == 3 && "emission value error");
                emission.x = std::stof(value[0]);
                emission.y = std::stof(value[1]);
                emission.z = std::stof(value[2]);
                lightsNames.push_back(name_->Value());
                lightsEmissions.push_back(emission);
            }
        }
    };

    class mayaParser
    {
    public:
        mayaParser() = default;
        mayaParser(const std::string &path)
        {
            parse(path);
        };
        ~mayaParser() = default;

        static Core::Scene *parse(const std::string &path)
        {
            std::cout << "start parsing file: " << path << "......";
            std::string mtlPath, objPath, xmlPath;
            // remove .mb
            std::string pathWithout_MB = path.substr(0, path.find_last_of("."));
            xmlPath = pathWithout_MB + ".xml";
            objPath = pathWithout_MB + ".obj";
            mayaXmlFile xmlFile{xmlPath};
            mayaObjFile objFile{objPath};
            auto scene = new Core::Scene(xmlFile.camera, xmlFile.settings);
            scene->materials = objFile.materialsRaw;
            for (auto &mesh : objFile.meshes)
                scene->meshes.push_back(mesh);
            for (auto &instance : objFile.instances)
                scene->instances.push_back(instance);
            std::vector<Core::Light> lights; // lights process
            for (int i = 0; i < xmlFile.lightsNames.size(); i++)
            {
                Core::Light light;
                std::string material_name = xmlFile.lightsNames[i]; // find the mesh who use this material
                auto it = std::find_if(objFile.materials.begin(), objFile.materials.end(), [&material_name](const tinyobj::material_t &mat)
                                       { return mat.name == material_name; });
                int materialId = it - objFile.materials.begin();
                int meshID = -1;
                assert(it != objFile.materials.end() && "material not found");
                for (int i = 0; i < scene->meshes.size(); i++)
                    if (scene->instances[i].mMaterialIndex == materialId)
                    {
                        meshID = scene->instances[i].mMeshIndex;
                        scene->instances[i].mActived = false; // set the instance to inactive
                        break;
                    }
                assert(meshID != -1 && "mesh id error");
                light.fromMesh2RectLight(scene->meshes[meshID], xmlFile.lightsEmissions[i]);
                lights.push_back(light);
            }

            // Core::Light light_added;
            // light_added.fromMesh2RectLight(scene->meshes[4], glm::vec3(1.0f, 1.0f, 1.0f));
            // scene->instances[4].mActived = false;
            // lights.push_back(light_added);
            for (auto &light : lights)
                scene->lights.push_back(light);
            std::cout << "Done!" << std::endl;
            return scene;
        }
    };
}
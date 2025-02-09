#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <utils/mathUtils.hpp>
#include <core/texture.hpp>
#include <glm/glm.hpp>

namespace scTracer::Core {
    enum class MaterialType {
        Diffuse,
        Conductor,
    };

    const std::vector<std::string> materialTypeStrings{
        "diffuse",
        "conductor"
    };

    MaterialType getMaterialType(const std::string& type);

    class Material {
    public:
        glm::vec3 baseColor;
        float anisotropic;

        glm::vec3 emission;
        float padding1;

        float metallic;
        float roughness;
        float subsurface;
        float specularTint;

        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatGloss;

        float specTrans;
        float ior;
        float mediumType;
        float mediumDensity;

        glm::vec3 mediumColor;
        float mediumAnisotropy;

        float baseColorTexId;
        float metallicRoughnessTexID;
        float normalmapTexID;
        float emissionmapTexID;

        float opacity;
        float alphaMode;
        float alphaCutoff;
        float padding2;
    };

    class MaterialRaw {

    public:
        MaterialRaw() = default;
        virtual ~MaterialRaw() = default;
        void typePreprocess() {
            if (mType == MaterialType::Diffuse) {
                roughness = 1.0f;
                metallic = 0.0f;
            }
            else if (mType == MaterialType::Conductor) {
                roughness = 0.0f;
                metallic = 1.0f;
            }
        }
        void printDebugInfo() {
            std::cout << "MaterialRaw Name: " << name << std::endl;
            std::cout << "Basecolor: " << basecolor.x << " " << basecolor.y << " " << basecolor.z << std::endl;
            std::cout << "MaterialRaw Type: " << materialTypeStrings[static_cast<int>(mType)] << std::endl;
            std::cout << "Roughness: " << roughness << std::endl;
            std::cout << "Metallic: " << metallic << std::endl;
        }

        Material getMaterial() {
            Material mat;
            mat.baseColor = basecolor;
            mat.roughness = roughness;
            mat.metallic = metallic;
            return mat;
        }

        //
        std::string name;
        glm::vec3 basecolor;
        //
        MaterialType mType;
        float roughness;
        float metallic;

    };






}
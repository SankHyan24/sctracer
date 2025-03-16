#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <utils/mathUtils.hpp>
#include <core/texture.hpp>
#include <glm/glm.hpp>

namespace scTracer::Core
{
    enum class MaterialType
    {
        Diffuse,
        Conductor,
    };

    const std::vector<std::string> materialTypeStrings{
        "diffuse",
        "conductor"};

    MaterialType getMaterialType(const std::string &type);

    class Material
    {
    public:
        glm::vec3 baseColor{1.0f, 1.0f, 1.0f};
        float anisotropic{0.0f};

        glm::vec3 emission{0.0f};
        float padding1;

        float metallic{0.0f};
        float roughness{0.5f};
        float subsurface{0.0f};
        float specularTint{0.0f};

        float sheen{0.0f};
        float sheenTint{0.0f};
        float clearcoat{0.0f};
        float clearcoatGloss{0.0f};

        float specTrans{0.0f};
        float ior{1.5f};
        float mediumType{0.0f};
        float mediumDensity{0.0f};

        glm::vec3 mediumColor{1.0f, 1.0f, 1.0f};
        float mediumAnisotropy{0.0f};

        float baseColorTexId{-1.0f};
        float metallicRoughnessTexID{-1.0f};
        float normalmapTexID{-1.0f};
        float emissionmapTexID{-1.0f};

        float opacity{1.0f};
        float alphaMode{0.0f};
        float alphaCutoff{0.0f};
        float padding2;
    };

    class MaterialRaw
    {

    public:
        MaterialRaw() = default;
        virtual ~MaterialRaw() = default;
        void typePreprocess()
        {
            if (mType == MaterialType::Diffuse)
            {
                roughness = 0.5f;
                metallic = 0.0f;
            }
            else if (mType == MaterialType::Conductor)
            {
                roughness = 0.0f;
                metallic = 1.0f;
            }
        }
        void printDebugInfo()
        {
            std::cout << "MaterialRaw Name: " << name << std::endl;
            std::cout << "Basecolor: " << basecolor.x << " " << basecolor.y << " " << basecolor.z << std::endl;
            std::cout << "MaterialRaw Type: " << materialTypeStrings[static_cast<int>(mType)] << std::endl;
            std::cout << "Roughness: " << roughness << std::endl;
            std::cout << "Metallic: " << metallic << std::endl;
        }

        Material getMaterial()
        {
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
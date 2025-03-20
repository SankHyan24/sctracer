#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <config.hpp>
#include <utils/mathUtils.hpp>
#include <core/texture.hpp>
#include <glm/glm.hpp>
// namespace scTracer::CPU
// {
//     float Luminance(glm::vec3 c);
// }
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

    const std::vector<std::string> materialSystemStrings{
        "PBRT",
        "PHONG",
        "DISNEY"};

    class MaterialRaw
    {

    public:
        enum MatSystem
        {
            PBRT,
            PHONG,
            DISNEY
        };

        MaterialRaw() = default;
        virtual ~MaterialRaw() = default;
        void printDebugInfo()
        {
            std::cout << "MaterialRaw Name: " << Config::LOG_GREEN << name << Config::LOG_RESET
                      << " using " << Config::LOG_GREEN << materialSystemStrings[static_cast<int>(mSystem)] << Config::LOG_RESET
                      << " system" << std::endl;
            if (mSystem != MatSystem::PHONG)
            {
                std::cout << "Basecolor: " << basecolor.x << " " << basecolor.y << " " << basecolor.z << std::endl;
                std::cout << "MaterialRaw Type: " << materialTypeStrings[static_cast<int>(mType)] << std::endl;
                std::cout << "Roughness: " << roughness << std::endl;
                std::cout << "Metallic: " << metallic << std::endl;
            }
            else
            {
                std::cout << "Kd: " << Kd.x << " " << Kd.y << " " << Kd.z << std::endl;
                std::cout << "Ks: " << Ks.x << " " << Ks.y << " " << Ks.z << std::endl;
                std::cout << "Tr: " << Tr.x << " " << Tr.y << " " << Tr.z << std::endl;
                std::cout << "Ka: " << Ka.x << " " << Ka.y << " " << Ka.z << std::endl;
                std::cout << "Ns: " << Ns << std::endl;
                std::cout << "Ni: " << Ni << std::endl;
            }
        }
        Material getMaterialFromPbrt()
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
            Material mat;
            mat.baseColor = basecolor;
            mat.roughness = roughness;
            mat.metallic = metallic;
            return mat;
        }

        Material getMaterialFromPhong()
        {
            // reference: blender/source/blender/io/wavefront_obj/importer/obj_import_mtl.cc
            /* Specular: average of Ks components. */
            float specular = (Ks.x + Ks.y + Ks.z) / 3.0f;
            specular = glm::clamp(specular, 0.0f, 1.0f);
            /* Roughness: map 0..1000 range to 1..0 and apply non-linearity. */
            float spec_exponent = Ns;
            if (Ns < 0)
                roughness = 1.0f;
            else
            {
                float clamped_ns = glm::clamp(Ns, 0.f, 1000.f);
                roughness = 1.0f - sqrt(clamped_ns / 1000.0f);
            }
            /* Metallic: average of `Ka` components. */
            metallic = (Ka.x + Ka.y + Ka.z) / 3.0f;
            Material mat;
            mat.baseColor = Kd;
            mat.roughness = roughness;
            mat.ior = Ni;
            mat.metallic = metallic;
            mat.emission = glm::vec3(0.0f);
            mat.opacity = Tr.x;
            mat.specularTint = specular * 0.08f;
            return mat;
        }
        Material getMaterialFromDisney()
        {
            Material mat;
            mat.baseColor = basecolor;
            mat.roughness = roughness;
            mat.metallic = metallic;
            return mat;
        }
        Material getMaterial()
        {
            switch (mSystem)
            {
            case MatSystem::PBRT:
                return getMaterialFromPbrt();
            case MatSystem::PHONG:
                return getMaterialFromPhong();
            default:
                return getMaterialFromDisney();
            }
        }
        //
        MatSystem mSystem;
        std::string name;
        union
        {
            glm::vec3 basecolor;
            glm::vec3 Kd{1.f};
        };
        glm::vec3 Ks{0.f};
        glm::vec3 Tr{1.0f};
        glm::vec3 Ka{1.0f};
        float Ns{1.0f};
        float Ni{1.0f};
        //
        MaterialType mType;
        float roughness;
        float metallic;

        int *baseColorTexId{nullptr};
    };

    class MaterialPhong
    {
    public:
        std::string name;
        glm::vec3 Kd{1.f};
        glm::vec3 Ks{0.f};
        glm::vec3 Tr{1.0f};
        float Ns{1.0f};
        float Ni{1.0f};
        void printDebugInfo() {}
        Material getMaterial()
        {
            Material mat;
            mat.baseColor = Kd;
            mat.roughness = sqrt(2.0 / (Ks.x + 2.0));
            mat.metallic = 1 - Ks.x;
            return mat;
        }
    };
}
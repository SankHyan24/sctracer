#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <utils/mathUtils.hpp>
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
        Material() = default;
        virtual ~Material() = default;
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
            std::cout << "Material Name: " << name << std::endl;
            std::cout << "Basecolor: " << basecolor.x << " " << basecolor.y << " " << basecolor.z << std::endl;
            std::cout << "Material Type: " << materialTypeStrings[static_cast<int>(mType)] << std::endl;
            std::cout << "Roughness: " << roughness << std::endl;
            std::cout << "Metallic: " << metallic << std::endl;
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
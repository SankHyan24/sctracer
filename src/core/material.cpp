#include <core/material.hpp>


namespace scTracer::Core {
    MaterialType getMaterialType(const std::string& type) {
        if (type == "diffuse")
            return MaterialType::Diffuse;
        if (type == "conductor")
            return MaterialType::Conductor;
        return MaterialType::Diffuse;
    }
}
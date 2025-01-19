#pragma once
#include <iostream>
namespace scTracer::Core {

    class Scene
    {
    public:
        Scene() {
            debug();
        };
        ~Scene() = default;
        void debug() {
            std::cout << "Scene::debug" << std::endl;
        }
    };

}
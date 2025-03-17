#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <tuple>
#include <core/scene.hpp>

namespace scTracer::Importer::Maya
{
    // this parser only works for the ZJU CG course's scene files
    class mayaParser
    {
        mayaParser() = default;
        mayaParser(const std::string &path)
        {
            parse(path);
        };
        ~mayaParser() = default;

        static Core::Scene *parse(const std::string &path){
            
        }
    };
}
#include <config.hpp>
namespace scTracer::Config
{
    const int default_width = 500;
    const int default_height = 500;
    const std::string shaderFolder = "shaders/";
    const std::string sceneFolder = "assets/";
    const std::string outputFolder = "./";

    const std::string LOG_RED = "\033[1;31m";
    const std::string LOG_GREEN = "\033[1;32m";
    const std::string LOG_YELLOW = "\033[1;33m";
    const std::string LOG_BLUE = "\033[1;34m";
    const std::string LOG_MAGENTA = "\033[1;35m";
    const std::string LOG_CYAN = "\033[1;36m";
    const std::string LOG_RESET = "\033[0m";
};
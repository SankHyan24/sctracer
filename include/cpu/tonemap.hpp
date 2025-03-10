#pragma once
#include <glm/glm.hpp>

namespace
{
    inline glm::vec3 luminance(glm::vec3 color)
    {
        return glm::vec3(0.2126f, 0.7152f, 0.0722f) * color;
    }
    inline glm::vec3 _toneMap(glm::vec3 color, float limit)
    {
        return color / (1.0f + luminance(color) / limit);
    }
}
namespace scTracer::CPU
{
    inline glm::vec4 toneMap(glm::vec4 color)
    {
        glm::vec3 rgb = glm::vec3(color.r, color.g, color.b);
        float alpha = color.a;
        rgb = _toneMap(rgb, 1.5f);
        rgb = pow(rgb, glm::vec3(1.0f / 2.2f));
        return glm::vec4(rgb, alpha);
    }

}
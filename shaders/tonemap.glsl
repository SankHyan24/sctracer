#version 450 core

out vec4 outCol;
in vec2 TexCoords;

uniform sampler2D pathTraceTexture;
uniform float invSampleCounter;

#include "include/globals.glsl"

vec3 Tonemap(in vec3 c, float limit)
{
    return c * 1.0 / (1.0 + Luminance(c) / limit);
}

void main() {
    vec4 col = texture(pathTraceTexture, TexCoords) * invSampleCounter;
    // vec4 col = texture(pathTraceTexture, TexCoords);
    vec3 color = col.rgb;
    float alpha = col.a;

    // color = Tonemap(color, 1.5); // tonmapping
    color = pow(color, vec3(1.0 / 2.2)); // gamma correction
    outCol = vec4(color, 1.0);
}
#version 460 core

out vec4 outCol;
in vec2 TexCoords;

uniform sampler2D accumTexture;
uniform float invSampleCounter;

void main() {
    outCol = (texture(accumTexture, TexCoords)) *invSampleCounter;
}
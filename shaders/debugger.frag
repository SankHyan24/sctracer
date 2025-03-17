#version 460 core
#include "include/uniforms.glsl"
#include "include/globals.glsl"

in vec2 TexCoords;             
out vec4 FragColor;

void main() {
    float fragDepth = gl_FragCoord.z;
    FragColor = vec4( 1.0,0.0,1.0,0.0); 
}
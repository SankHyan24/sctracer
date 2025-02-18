#version 450 core

in vec2 TexCoords;             
out vec4 FragColor;

void main() {
    float fragDepth = gl_FragCoord.z;
    FragColor = vec4( 1.0); 
}
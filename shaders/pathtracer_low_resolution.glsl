#version 460 core

in vec2 TexCoords;             
out vec4 FragColor;

void main() {
    float fragDepth = gl_FragCoord.z;
    FragColor = vec4(1,1,0.3, 1.0); 
}
#version 450 core

in vec3 ourColor;
in vec2 TexCoords;             
out vec4 FragColor;

void main() {
    float fragDepth = gl_FragCoord.z;
    FragColor = vec4(ourColor, 1.0); 
    FragColor = vec4(1,1,0.3, 1.0); 
}
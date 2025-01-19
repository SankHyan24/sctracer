#version 450 core

uniform sampler2D depthBuffer; 
in vec3 ourColor;
in vec2 TexCoords;             
out vec4 FragColor;

void main() {
    float fragDepth = gl_FragCoord.z;

    float zBufferDepth = texture(depthBuffer, TexCoords).r;

    FragColor = vec4(ourColor, 1.0); 

}
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 view; // 视图矩阵
uniform mat4 projection; // 投影矩阵
uniform mat4 model; // 模型矩阵
out vec3 ourColor;
out vec2 TexCoords; 
out vec3 fragNormal;            
// 

void main()
{
    vec4 clipSpace = projection * view * model * vec4(aPosition, 1.0);
    gl_Position = clipSpace;
    TexCoords = (clipSpace.xy / clipSpace.w + 1.0) / 2.0;
    // ourColor = aNormal;
    // ourColor = aPosition;
}
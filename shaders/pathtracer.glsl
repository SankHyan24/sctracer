#include "include/uniforms.glsl"
#include "include/globals.glsl"

out vec4 color;
in vec2 TexCoords;

void main() {
    float fragDepth = gl_FragCoord.z;
    vec4 accumColor = texture(accumTexture, TexCoords);
    color = vec4(1,0.1,0.3, 1.0)+accumColor; 
}
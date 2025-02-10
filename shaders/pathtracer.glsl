#include "include/uniforms.glsl"
#include "include/globals.glsl"
#include "include/intersection.glsl"
#include "include/closest_hit.glsl"
#include "include/traceray.glsl"

out vec4 color;
in vec2 TexCoords;

void main() {

    vec2 coords = TexCoords;
    InitRNG(gl_FragCoord.xy, frameNum);
    // float r1 = 2.0 * rand();
    // float r2 = 2.0 * rand();

    // vec2 jitter;
    // jitter.x = r1 < 1.0 ? sqrt(r1) - 1.0 : 1.0 - sqrt(2.0 - r1);
    // jitter.y = r2 < 1.0 ? sqrt(r2) - 1.0 : 1.0 - sqrt(2.0 - r2);

    // jitter /= (resolution * 0.5);
    // vec2 d = (TexCoords * 2.0 - 1.0) + jitter;
    vec2 d = (TexCoords * 2.0 - 1.0);
    float scale = tan(camera.fov * 0.5);

    d.y *= resolution.y / resolution.x * scale;
    d.x *= scale;
    vec3 rayDir = normalize(d.x * camera.right + d.y * camera.up + camera.forward);


    vec3 focalPoint = camera.focalDist * rayDir;
    float cam_r1 = rand() * TWO_PI;
    float cam_r2 = rand() * camera.aperture;
    vec3 randomAperturePos = (cos(cam_r1) * camera.right + sin(cam_r1) * camera.up) * sqrt(cam_r2);
    vec3 finalRayDir = normalize(focalPoint );


    Ray ray = Ray(camera.position, finalRayDir);







    vec4 accumColor = texture(accumTexture, TexCoords);
    vec4 pixelColor = traceRay(ray);



    color = pixelColor ; 
}
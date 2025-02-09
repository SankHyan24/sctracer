uniform bool isCameraMoving;
uniform vec3 randomVector;
uniform vec2 resolution;

uniform sampler2D accumTexture;
uniform samplerBuffer BVH;
uniform isamplerBuffer vertexIndicesTex;
uniform samplerBuffer verticesTex;
uniform samplerBuffer normalsTex;
uniform samplerBuffer uvsTex;
uniform sampler2D materialsTex;
uniform sampler2D transformsTex;
uniform sampler2D lightsTex;
uniform sampler2DArray textureMapsArrayTex;


uniform vec3 uniformLightCol;
uniform int numOfLights;
uniform int maxDepth;
uniform int topBVHIndex;
uniform int frameNum;
uniform float roughnessMollificationAmt;
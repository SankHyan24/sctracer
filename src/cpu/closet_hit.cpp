#include <cpu/integrator.hpp>

namespace scTracer::CPU
{
    bool Integrator::ClosestHit(Ray r, State &state, LightSampleRec lightSample, glm::vec3 &debugger)
    {
        float t = INF;
        float d;

        // intersect with BVH
        int stack[64];
        int ptr = 0;
        stack[ptr++] = -1;
        int index = uniforms.topBVHIndex;
        float leftHit = 0.0f, rightHit = 0.0f;

        int currMatID = 0;
        bool BLAS = false;

        glm::ivec3 triID = glm::ivec3(-1);
        glm::mat4 transMat;
        glm::mat4 transform;
        glm::vec3 bary;
        glm::vec4 vert0, vert1, vert2;

        Ray rTrans;
        rTrans.origin = r.origin;
        rTrans.direction = r.direction;

        while (index != -1)
        {
            glm::ivec3 LRLeaf = mScene->bvhFlattor.flattenedNodes[index].LeftRightLeaf;

            int leftIndex = LRLeaf.x;
            int rightIndex = LRLeaf.y;
            int leaf = LRLeaf.z;

            if (leaf > 0) // Leaf node of BLAS
            {
                // std::cout << "Leaf node of BLAS" << std::endl;
                debugger += glm::vec3(0., 0., 0.2);
                for (int i = 0; i < rightIndex; i++) // Loop through tris
                {
                    glm::ivec3 vertIndices = glm::ivec3(mScene->sceneTriIndices[leftIndex + i * 3 + 0],
                                                        mScene->sceneTriIndices[leftIndex + i * 3 + 1],
                                                        mScene->sceneTriIndices[leftIndex + i * 3 + 2]);
                    glm::vec4 v0 = glm::vec4(mScene->sceneVertices[vertIndices.x], mScene->sceneMeshUvs[vertIndices.x].x);
                    glm::vec4 v1 = glm::vec4(mScene->sceneVertices[vertIndices.y], mScene->sceneMeshUvs[vertIndices.y].x);
                    glm::vec4 v2 = glm::vec4(mScene->sceneVertices[vertIndices.z], mScene->sceneMeshUvs[vertIndices.z].x);

                    glm::vec3 v0xyz = glm::vec3(v0.x, v0.y, v0.z);
                    glm::vec3 v1xyz = glm::vec3(v1.x, v1.y, v1.z);
                    glm::vec3 v2xyz = glm::vec3(v2.x, v2.y, v2.z);
                    glm::vec3 e0 = v1xyz - v0xyz;
                    glm::vec3 e1 = v2xyz - v0xyz;
                    glm::vec3 pv = glm::cross(rTrans.direction, e1);
                    float det = glm::dot(e0, pv);

                    glm::vec3 tv = rTrans.origin - v0xyz;
                    glm::vec3 qv = glm::cross(tv, e0);

                    glm::vec4 uvt;
                    uvt.x = glm::dot(tv, pv);
                    uvt.y = glm::dot(rTrans.direction, qv);
                    uvt.z = glm::dot(e1, qv);
                    glm::vec3 uvt3 = glm::vec3(uvt.x, uvt.y, uvt.z);
                    uvt.x = uvt.x / det;
                    uvt.y = uvt.y / det;
                    uvt.z = uvt.z / det;
                    uvt.w = 1.0 - uvt.x - uvt.y;

                    if (glm::all(glm::greaterThanEqual(uvt, glm::vec4(0.0))) && uvt.z < t)
                    {
                        t = uvt.z;
                        triID = vertIndices;
                        state.matID = currMatID;
                        // bary = uvt.wxy;
                        bary = glm::vec3(uvt.w, uvt.x, uvt.y);
                        vert0 = v0, vert1 = v1, vert2 = v2;
                        transform = transMat;
                    }
                }
            }
            else if (leaf < 0) // Leaf node of TLAS
            {
                // std::cout << "Leaf node of TLAS" << std::endl;
                debugger += glm::vec3(0., 0.2, 0);

                glm::mat4 transform = mScene->transforms[(-leaf - 1)];

                transMat = transform;

                rTrans.origin = glm::vec3(glm::inverse(transMat) * glm::vec4(r.origin, 1.0));
                rTrans.direction = glm::vec3(glm::inverse(transMat) * glm::vec4(r.direction, 0.0));

                // Add a marker. We'll return to this spot after we've traversed the entire BLAS
                stack[ptr++] = -1;
                index = leftIndex;
                BLAS = true;
                currMatID = rightIndex;
                continue;
            }
            else
            {
                debugger += glm::vec3(0.2, 0., 0);

                leftHit = AABBIntersect(mScene->bvhFlattor.flattenedNodes[leftIndex].boundsmin, mScene->bvhFlattor.flattenedNodes[leftIndex].boundsmax, rTrans);
                rightHit = AABBIntersect(mScene->bvhFlattor.flattenedNodes[rightIndex].boundsmin, mScene->bvhFlattor.flattenedNodes[rightIndex].boundsmax, rTrans);

                if (leftHit > 0.0 && rightHit > 0.0)
                {
                    int deferred = -1;
                    if (leftHit > rightHit)
                    {
                        index = rightIndex;
                        deferred = leftIndex;
                    }
                    else
                    {
                        index = leftIndex;
                        deferred = rightIndex;
                    }

                    stack[ptr++] = deferred;
                    continue;
                }
                else if (leftHit > 0.)
                {
                    index = leftIndex;
                    continue;
                }
                else if (rightHit > 0.)
                {
                    index = rightIndex;
                    continue;
                }
            }
            index = stack[--ptr];

            // If we've traversed the entire BLAS then switch to back to TLAS and resume where we left off
            if (BLAS && index == -1)
            {
                BLAS = false;

                index = stack[--ptr];

                rTrans.origin = r.origin;
                rTrans.direction = r.direction;
            }
        }

        return false;
    }
}

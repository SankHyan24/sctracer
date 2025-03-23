#include <core/mesh.hpp>
namespace scTracer::Core
{
    void Mesh::BuildBVH()
    {
        const int triangleNumber = indices.size();
        std::vector<BVH::BoundingBox> bounds(triangleNumber);
        for (int i = 0; i < triangleNumber; i++)
        {
            glm::vec3 v0 = vertices[indices[i].x];
            glm::vec3 v1 = vertices[indices[i].y];
            glm::vec3 v2 = vertices[indices[i].z];
            bounds[i].grow(v0);
            bounds[i].grow(v1);
            bounds[i].grow(v2);
        }
        bvh->build(&bounds[0], triangleNumber);
    }

}
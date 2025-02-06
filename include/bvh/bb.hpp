#pragma once
#include <cmath>

#include <glm/glm.hpp>

namespace scTracer::BVH {
    class BoundingBox {
    public:
        glm::vec3 pmin;
        glm::vec3 pmax;
        BoundingBox() :pmin(glm::vec3(std::numeric_limits<float>::infinity())), pmax(glm::vec3(-std::numeric_limits<float>::infinity())) {}

        BoundingBox(const glm::vec3& p) : pmin(p), pmax(p) {}
        BoundingBox(const glm::vec3& p1, const glm::vec3& p2) {
            pmin = glm::min(p1, p2);
            pmax = glm::max(p1, p2);
        }
        // center
        glm::vec3 centroid() const;
        glm::vec3 extents() const;

        bool contains(const glm::vec3& p) const;

        inline int maxDimension() const {
            glm::vec3 d = extents();
            if (d.x > d.y && d.x > d.z) return 0;
            else if (d.y > d.z) return 1;
            else return 2;
        }

        float surfaceArea() const;

        const glm::vec3& operator [] (int i) const { return *(&pmin + i); }

        // grow
        void grow(const glm::vec3& p);
        void grow(const BoundingBox& bb);
        const BoundingBox operator + (const glm::vec3& p) const;
        BoundingBox& operator += (const glm::vec3& p);
    };

    // bbox bboxunion(bbox const& box1, bbox const& box2);
    // bbox intersection(bbox const& box1, bbox const& box2);
    // void intersection(bbox const& box1, bbox const& box2, bbox& box);
    // bool intersects(bbox const& box1, bbox const& box2);
    // bool contains(bbox const& box1, bbox const& box2);

    BoundingBox bboxUnion(const BoundingBox& box1, const BoundingBox& box2);
    BoundingBox intersection(const BoundingBox& box1, const BoundingBox& box2);
    void intersection(const BoundingBox& box1, const BoundingBox& box2, BoundingBox& box);
    bool intersects(const BoundingBox& box1, const BoundingBox& box2);
    bool contains(const BoundingBox& box1, const BoundingBox& box2);

}
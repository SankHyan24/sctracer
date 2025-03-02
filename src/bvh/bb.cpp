#include <bvh/bb.hpp>
// BVH part Code learn from https://github.com/knightcrawler25/GLSL-PathTracer

namespace scTracer::BVH
{

    const float BBOX_INTERSECTION_EPS = 0.f;

    glm::vec3 BoundingBox::centroid() const
    {
        return (pmin + pmax) / 2.0f;
    }

    glm::vec3 BoundingBox::extents() const
    {
        return pmax - pmin;
    }

    bool BoundingBox::contains(const glm::vec3 &p) const
    {
        glm::vec3 radius = extents() * 0.5f;

        return std::abs(centroid().x - p.x) <= radius.x &&
               fabs(centroid().y - p.y) <= radius.y &&
               fabs(centroid().z - p.z) <= radius.z;
    }

    float BoundingBox::surfaceArea() const
    {
        glm::vec3 d = extents();
        return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    void BoundingBox::grow(const glm::vec3 &p)
    {
        pmin = glm::min(pmin, p);
        pmax = glm::max(pmax, p);
    }

    void BoundingBox::grow(const BoundingBox &bb)
    {
        pmin = glm::min(pmin, bb.pmin);
        pmax = glm::max(pmax, bb.pmax);
    }

    const BoundingBox BoundingBox::operator+(const glm::vec3 &p) const
    {
        BoundingBox res = *this;
        res.grow(p);
        return res;
    }

    BoundingBox &BoundingBox::operator+=(const glm::vec3 &p)
    {
        grow(p);
        return *this;
    }

    BoundingBox bboxUnion(const BoundingBox &box1, const BoundingBox &box2)
    {
        BoundingBox res = box1;
        res.pmin = glm::min(box1.pmin, box2.pmin);
        res.pmax = glm::max(box1.pmax, box2.pmax);
        return res;
    }

    BoundingBox intersection(const BoundingBox &box1, const BoundingBox &box2)
    {
        BoundingBox res = box1;
        res.pmin = glm::max(box1.pmin, box2.pmin);
        res.pmax = glm::min(box1.pmax, box2.pmax);
        return res;
    }

    void intersection(const BoundingBox &box1, const BoundingBox &box2, BoundingBox &box)
    {
        box.pmin = glm::max(box1.pmin, box2.pmin);
        box.pmax = glm::min(box1.pmax, box2.pmax);
    }

    bool intersects(const BoundingBox &box1, const BoundingBox &box2)
    {
        glm::vec3 b1c = box1.centroid();
        glm::vec3 b1r = box1.extents() * 0.5f;
        glm::vec3 b2c = box2.centroid();
        glm::vec3 b2r = box2.extents() * 0.5f;
        return (fabs(b2c.x - b1c.x) - (b1r.x + b2r.x)) <= BBOX_INTERSECTION_EPS &&
               (fabs(b2c.y - b1c.y) - (b1r.y + b2r.y)) <= BBOX_INTERSECTION_EPS &&
               (fabs(b2c.z - b1c.z) - (b1r.z + b2r.z)) <= BBOX_INTERSECTION_EPS;
    }

    bool contains(const BoundingBox &box1, const BoundingBox &box2)
    {
        return box1.contains(box2.pmin) && box1.contains(box2.pmax);
    }
}
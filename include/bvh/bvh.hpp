#pragma once
#include <bvh/bb.hpp>

namespace scTracer::BVH {
    class BvhStructure {
    public:
        BvhStructure(float traversal_cost, int num_bins = 64, bool usesah = false)
            : m_root(nullptr)
            , m_num_bins(num_bins)
            , m_usesah(usesah)
            , m_height(0)
            , m_traversal_cost(traversal_cost) {
        }
        ~BvhStructure() = default;

        //TODO: finish it
        struct Node;




        // Bounding box which containing all primitives
        BoundingBox m_bounds;
        // Root node
        Node* m_root;
        // SAH flag
        bool m_usesah;
        // Tree height
        int m_height;
        // Node traversal cost
        float m_traversal_cost;
        // Number of spatial bins to use for SAH
        int m_num_bins;

    private:
    };

    struct BvhStructure::Node
    {
    };
}
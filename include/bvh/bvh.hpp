#pragma once
#include <iostream>
#include <bvh/bb.hpp>
#include <config.hpp>

namespace scTracer::BVH {
    class BvhStructure {
    public:
        struct Node;
        enum NodeType
        {
            kInternal,
            kLeaf
        };

        BvhStructure(float traversal_cost = 2.0f, int num_bins = 64, bool usesah = false)
            : mRoot(nullptr)
            , mSahBinsNum(num_bins)
            , mUseSah(usesah)
            , mHeight(0)
            , mTraversalCost(traversal_cost) {
        }
        ~BvhStructure() = default;

        // Get
        const BoundingBox& getWorldBounds() const;
        inline int const* getIndices() const { return &mPackedIndices[0]; }
        inline size_t getNumIndices() const { return mPackedIndices.size(); }
        inline Node const* getRoot() const { return mRoot; }
        inline int getHeight() const { return mHeight; }


        // Print BVH statistics
        virtual void printStatistics(std::ostream& os) const;

        // Build
        void build(const BoundingBox* bounds, int numbounds);

        // Bounding box which containing all primitives
        BoundingBox mTopBoundingBox;
        // Root node
        Node* mRoot;
        // SAH flag
        bool mUseSah;
        // Tree height
        int mHeight;
        // Node traversal cost
        float mTraversalCost;
        // Number of spatial bins to use for SAH
        int mSahBinsNum;

        // Bvh nodes
        std::vector<Node> mNodes;
        // Identifiers of leaf primitives
        std::vector<int> mIndices;
        // Node allocator counter, atomic for thread safety
        std::atomic<int> mNodeCount;
        // Identifiers of leaf primitives
        std::vector<int> mPackedIndices;
    protected:
        virtual void _build(const BoundingBox* bounds, int numbounds);
        virtual Node* _allocateNode();
        virtual void  _initNodeAllocator(size_t maxnum);

        struct SplitRequest
        {
            // Starting index of a request
            int startidx;
            // Number of primitives
            int numprims;
            // Root node
            Node** ptr;
            // Bounding box
            BoundingBox bounds;
            // Centroid bounds
            BoundingBox centroid_bounds;
            // Level
            int level;
            // Node index
            int index;
        };

        struct SahSplit
        {
            int dim;
            float split;
            float sah;
            float overlap;
        };

        void _buildNode(const SplitRequest& req, const BoundingBox* bounds, const glm::vec3* centroids, int* primindices);
        SahSplit _findSahSplit(const SplitRequest& req, const BoundingBox* bounds, const glm::vec3* centroids, int* primindices) const;
    private:
    };

    struct BvhStructure::Node
    {
        BoundingBox bb; // world space bounding box
        NodeType type;
        Uint index;
        union
        {
            // For internal nodes: left and right children
            struct
            {
                Node* leftChild;
                Node* rightChild;
            };

            // For leaves: starting primitive index and number of primitives
            struct
            {
                int startIndex;
                int primsNum;
            };
        };
    };
}
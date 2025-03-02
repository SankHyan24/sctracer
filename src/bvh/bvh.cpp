#include <bvh/bvh.hpp>
#include <numeric>

namespace scTracer::BVH
{
    static int constexpr kMaxPrimitivesPerLeaf = 1;
    static bool is_nan(float v) { return v != v; }

    const BoundingBox &BvhStructure::getWorldBounds() const { return mTopBoundingBox; }

    void BvhStructure::build(const BoundingBox *bounds, int numbounds)
    {
        for (int i = 0; i < numbounds; ++i)
            mTopBoundingBox.grow(bounds[i]);
        _build(bounds, numbounds);
    }

    void BvhStructure::printStatistics(std::ostream &os) const
    {
        os << "Class name: " << "Bvh\n";
        os << "SAH: " << (mUseSah ? "enabled\n" : "disabled\n");
        os << "SAH bins: " << mSahBinsNum << "\n";
        os << "Number of triangles: " << mIndices.size() << "\n";
        os << "Number of nodes: " << mNodeCount << "\n";
        os << "Tree height: " << getHeight() << "\n";
    }

    void BvhStructure::_build(const BoundingBox *bounds, int numbounds)
    {
        _initNodeAllocator(2 * numbounds - 1);
        std::vector<glm::vec3> centroids(numbounds);
        mIndices.resize(numbounds);
        std::iota(mIndices.begin(), mIndices.end(), 0);

        BoundingBox centroid_bounds;
        for (size_t i = 0; i < static_cast<size_t>(numbounds); ++i)
        {
            glm::vec3 c = bounds[i].centroid();
            centroid_bounds.grow(c);
            centroids[i] = c;
        }

        SplitRequest init = {0, numbounds, nullptr, mTopBoundingBox, centroid_bounds, 0, 1};
        _buildNode(init, bounds, &centroids[0], &mIndices[0]);

        mRoot = &mNodes[0];
    }

    void BvhStructure::_initNodeAllocator(size_t maxnum)
    {
        mNodes.resize(maxnum);
        mNodeCount = 0;
    }

    BvhStructure::Node *BvhStructure::_allocateNode()
    {
        return &mNodes[mNodeCount++];
    }

    void BvhStructure::_buildNode(const SplitRequest &req, const BoundingBox *bounds, const glm::vec3 *centroids, int *primindices)
    {
        mHeight = std::max(mHeight, req.level);
        Node *node = _allocateNode();
        node->bb = req.bounds;
        node->index = req.index;

        if (req.numprims < 2) // Create leaf node if we have enough prims
        {
            node->type = kLeaf;
            node->startIndex = static_cast<int>(mPackedIndices.size());
            node->primsNum = req.numprims;

            for (auto i = 0; i < req.numprims; i++)
                mPackedIndices.push_back(primindices[req.startidx + i]);
        }
        else
        {
            // Choose the maximum extent
            int axis = req.centroid_bounds.maxDimension();
            float border = req.centroid_bounds.centroid()[axis];

            if (mUseSah)
            {
                SahSplit ss = _findSahSplit(req, bounds, centroids, primindices);

                if (!is_nan(ss.split))
                {
                    axis = ss.dim;
                    border = ss.split;
                    if (req.numprims < ss.sah && req.numprims < kMaxPrimitivesPerLeaf)
                    {
                        node->type = kLeaf;
                        node->startIndex = static_cast<int>(mPackedIndices.size());
                        node->primsNum = req.numprims;
                        for (auto i = 0; i < req.numprims; ++i)
                            mPackedIndices.push_back(primindices[req.startidx + i]);
                        if (req.ptr)
                            *req.ptr = node;
                        return;
                    }
                }
            }

            node->type = kInternal;

            // Start partitioning and updating extents for children at the same time
            BoundingBox leftbounds, rightbounds, leftcentroid_bounds, rightcentroid_bounds;
            int splitidx = req.startidx;

            bool near2far = (req.numprims + req.startidx) & 0x1;

            if (req.centroid_bounds.extents()[axis] > 0.f)
            {
                auto first = req.startidx;
                auto last = req.startidx + req.numprims;

                if (near2far)
                {
                    while (true)
                    {
                        while ((first != last) &&
                               centroids[primindices[first]][axis] < border)
                        {
                            leftbounds.grow(bounds[primindices[first]]);
                            leftcentroid_bounds.grow(centroids[primindices[first]]);
                            ++first;
                        }

                        if (first == last--)
                            break;

                        rightbounds.grow(bounds[primindices[first]]);
                        rightcentroid_bounds.grow(centroids[primindices[first]]);

                        while ((first != last) &&
                               centroids[primindices[last]][axis] >= border)
                        {
                            rightbounds.grow(bounds[primindices[last]]);
                            rightcentroid_bounds.grow(centroids[primindices[last]]);
                            --last;
                        }

                        if (first == last)
                            break;

                        leftbounds.grow(bounds[primindices[last]]);
                        leftcentroid_bounds.grow(centroids[primindices[last]]);

                        std::swap(primindices[first++], primindices[last]);
                    }
                }
                else
                {
                    while (true)
                    {
                        while ((first != last) &&
                               centroids[primindices[first]][axis] >= border)
                        {
                            leftbounds.grow(bounds[primindices[first]]);
                            leftcentroid_bounds.grow(centroids[primindices[first]]);
                            ++first;
                        }

                        if (first == last--)
                            break;

                        rightbounds.grow(bounds[primindices[first]]);
                        rightcentroid_bounds.grow(centroids[primindices[first]]);

                        while ((first != last) &&
                               centroids[primindices[last]][axis] < border)
                        {
                            rightbounds.grow(bounds[primindices[last]]);
                            rightcentroid_bounds.grow(centroids[primindices[last]]);
                            --last;
                        }

                        if (first == last)
                            break;

                        leftbounds.grow(bounds[primindices[last]]);
                        leftcentroid_bounds.grow(centroids[primindices[last]]);

                        std::swap(primindices[first++], primindices[last]);
                    }
                }

                splitidx = first;
            }

            if (splitidx == req.startidx || splitidx == req.startidx + req.numprims)
            {
                splitidx = req.startidx + (req.numprims >> 1);

                for (int i = req.startidx; i < splitidx; ++i)
                {
                    leftbounds.grow(bounds[primindices[i]]);
                    leftcentroid_bounds.grow(centroids[primindices[i]]);
                }

                for (int i = splitidx; i < req.startidx + req.numprims; ++i)
                {
                    rightbounds.grow(bounds[primindices[i]]);
                    rightcentroid_bounds.grow(centroids[primindices[i]]);
                }
            }

            SplitRequest leftrequest = {req.startidx, splitidx - req.startidx, &node->leftChild, leftbounds, leftcentroid_bounds, req.level + 1, (req.index << 1)};
            _buildNode(leftrequest, bounds, centroids, primindices);
            SplitRequest rightrequest = {splitidx, req.numprims - (splitidx - req.startidx), &node->rightChild, rightbounds, rightcentroid_bounds, req.level + 1, (req.index << 1) + 1};
            _buildNode(rightrequest, bounds, centroids, primindices);
        }

        // Set parent ptr if any
        if (req.ptr)
            *req.ptr = node;
    }

    BvhStructure::SahSplit BvhStructure::_findSahSplit(const SplitRequest &req, const BoundingBox *bounds, const glm::vec3 *centroids, int *primindices) const
    {
        assert(false && "Not implemented yet");
        return SahSplit();
    }
}
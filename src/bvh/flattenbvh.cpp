#include <bvh/flattenbvh.hpp>

namespace scTracer::BVH {
    int BVHFlattor::_flattenBLASNode(const BVH::BvhStructure::Node* node) {
        // node->print(std::cerr);
        int index = currentNodeIndex;
        BoundingBox bounds = node->bb;
        flattenedNodes[index].boundsmin = bounds.pmin;
        flattenedNodes[index].boundsmax = bounds.pmax;
        // flattenedNodes[index].isLeaf = 0;
        flattenedNodes[index].LeftRightLeaf.z = 0;
        if (node->type == BVH::BvhStructure::NodeType::kLeaf) {
            flattenedNodes[index].LeftRightLeaf.z = 1;
            flattenedNodes[index].LeftRightLeaf.x = currentTriIndex + node->startIndex;
            flattenedNodes[index].LeftRightLeaf.y = node->primsNum;
        }
        else {
            currentNodeIndex++;
            flattenedNodes[index].LeftRightLeaf.x = _flattenBLASNode(node->leftChild);
            currentNodeIndex++;
            flattenedNodes[index].LeftRightLeaf.y = _flattenBLASNode(node->rightChild);
        }
        return index;
    }

    int BVHFlattor::_flattenTLASNode(const BVH::BvhStructure::Node* node) {
        // node->print(std::cerr);
        int index = currentNodeIndex;
        BoundingBox bounds = node->bb;
        flattenedNodes[index].boundsmin = bounds.pmin;
        flattenedNodes[index].boundsmax = bounds.pmax;
        flattenedNodes[index].LeftRightLeaf.z = 0;
        if (node->type == BVH::BvhStructure::NodeType::kLeaf) {
            int instanceIndex = topLevelBvh->mPackedIndices[node->startIndex];
            int meshIndex = instances[instanceIndex].mMeshIndex;
            int materialIndex = instances[instanceIndex].mMaterialIndex;
            flattenedNodes[index].LeftRightLeaf.x = bvhRootStartIndices[meshIndex];
            flattenedNodes[index].LeftRightLeaf.y = materialIndex;
            flattenedNodes[index].LeftRightLeaf.z = -instanceIndex - 1;// avoid 0 n 1
        }
        else {
            currentNodeIndex++;
            flattenedNodes[index].LeftRightLeaf.x = _flattenTLASNode(node->leftChild);
            currentNodeIndex++;
            flattenedNodes[index].LeftRightLeaf.y = _flattenTLASNode(node->rightChild);
        }
        return index;
    }

    void BVHFlattor::_flattenBLAS() {
        int nodeCnt = 0;
        for (int i = 0; i < meshes.size(); i++)
            nodeCnt += meshes[i]->bvh->mNodeCount;
        topLevelIndex = nodeCnt;
        // make rooms
        nodeCnt += instances.size() * 2;
        flattenedNodes.resize(nodeCnt);
        int bvhRootIndex = 0;
        currentTriIndex = 0;
        for (int i = 0; i < meshes.size(); i++)
        {
            auto mesh = meshes[i];
            currentNodeIndex = bvhRootIndex;
            bvhRootStartIndices.push_back(bvhRootIndex);
            bvhRootIndex += mesh->bvh->mNodeCount;
            _flattenBLASNode(mesh->bvh->getRoot());
            currentTriIndex += mesh->bvh->getNumIndices();
        }
    }

    void BVHFlattor::_flattenTLAS() {
        currentNodeIndex = topLevelIndex;
        _flattenTLASNode(topLevelBvh->getRoot());
    }

    void BVHFlattor::updateTLAS(const BvhStructure* topLevelBvh, const std::vector<Core::Instance>& instances) {
        this->topLevelBvh = topLevelBvh;
        this->instances = instances;
        currentNodeIndex = topLevelIndex;
        _flattenTLASNode(topLevelBvh->getRoot());
    }

    void BVHFlattor::flatten(const BvhStructure* topLevelBvh, const std::vector<Core::Mesh*>& meshes, const std::vector<Core::Instance>& instances) {
        this->topLevelBvh = topLevelBvh;
        this->meshes = meshes;
        this->instances = instances;
        _flattenBLAS();
        _flattenTLAS();
    }

    void printFlatNode(const BVHFlattor::FlatNode& node, std::ostream& os) {
        os << "FlatNode: " << std::endl;
        os << "Bounds min: " << node.boundsmin.x << " " << node.boundsmin.y << " " << node.boundsmin.z << std::endl;
        os << "Bounds max: " << node.boundsmax.x << " " << node.boundsmax.y << " " << node.boundsmax.z << std::endl;
        os << "LeftRightLeaf: " << node.LeftRightLeaf.x << " " << node.LeftRightLeaf.y << " " << node.LeftRightLeaf.z << std::endl;
    }
}
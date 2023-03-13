#include "bvh.h"

#include <nox/maths/bounding_box.h>

#include <algorithm>

namespace NOXPT {

    struct BVHTriangleInfo {
        BVHTriangleInfo() = default;
        BVHTriangleInfo(const size_t index, const KernelTypes::Triangle &triangle) : index(index),
                                                                                     bounds({std::min({triangle.v0.position.x, triangle.v1.position.x, triangle.v2.position.x}),
                                                                                             std::min({triangle.v0.position.y, triangle.v1.position.y, triangle.v2.position.y}),
                                                                                             std::min({triangle.v0.position.z, triangle.v1.position.z, triangle.v2.position.z})},
                                                                                            {std::max({triangle.v0.position.x, triangle.v1.position.x, triangle.v2.position.x}),
                                                                                             std::max({triangle.v0.position.y, triangle.v1.position.y, triangle.v2.position.y}),
                                                                                             std::max({triangle.v0.position.z, triangle.v1.position.z, triangle.v2.position.z})}) {}

        size_t index{};
        NOX::BoundingBox bounds{};
    };

    struct BVHNode {
        void initLeaf(const uint32_t index, const uint32_t count, const NOX::BoundingBox &box) {
            bounds = box;
            firstTriangleOffset = index;
            triangleCount = count;
        }

        void initNode(const uint8_t axis, BVHNode *left, BVHNode *right) {
            bounds = {left->bounds, right->bounds};
            leftChild = left;
            rightChild = right;
            splitAxis = axis;
        }

        NOX::BoundingBox bounds{};
        BVHNode *leftChild{nullptr};
        BVHNode *rightChild{nullptr};
        uint32_t firstTriangleOffset{};
        uint32_t triangleCount{};
        uint32_t splitAxis{};
    };

    struct BucketInfo {
        uint32_t count{0u};
        NOX::BoundingBox bounds{};
    };

    void BVH::build(const std::vector<KernelTypes::Triangle> &triangles) {
        m_orderedTriangles.reserve(triangles.size());
        std::vector<BVHTriangleInfo> trianglesInfo(triangles.size());
        for (size_t i = 0; i < triangles.size(); i++) {
            trianglesInfo[i] = {i, triangles[i]};
        }

        uint32_t totalNodes = 0;
        BVHNode *root = subdivide(triangles, trianglesInfo, 0, static_cast<uint32_t>(triangles.size()), totalNodes);

        uint32_t offset = 0;
        m_nodes.resize(totalNodes);
        buildNodesBuffer(root, offset);
        cleanupNodes(root);
    }

    BVHNode *BVH::subdivide(const std::vector<KernelTypes::Triangle> &triangles, std::vector<BVHTriangleInfo> &trianglesInfo, const uint32_t start, const uint32_t end, uint32_t &totalNodes) {
        BVHNode *node = new BVHNode();
        totalNodes++;

        NOX::BoundingBox bounds{};
        for (auto i = start; i < end; i++) {
            bounds.grow(trianglesInfo[i].bounds);
        }

        uint32_t trianglesCount = end - start;
        if (trianglesCount == 1u) {
            auto offset = m_orderedTriangles.size();
            for (auto i = start; i < end; i++) {
                m_orderedTriangles.push_back(triangles[trianglesInfo[i].index]);
            }

            node->initLeaf(static_cast<uint32_t>(offset), trianglesCount, bounds);
            return node;
        } else {
            NOX::BoundingBox centroidBounds{};
            for (auto i = start; i < end; i++) {
                centroidBounds.grow(trianglesInfo[i].bounds.centroid());
            }

            auto splitAxis = centroidBounds.maximumExtentAxis();
            auto mid = (start + end) / 2;
            if (centroidBounds.minimum()[splitAxis] == centroidBounds.maximum()[splitAxis]) {
                auto offset = m_orderedTriangles.size();
                for (auto i = start; i < end; i++) {
                    m_orderedTriangles.push_back(triangles[trianglesInfo[i].index]);
                }

                node->initLeaf(static_cast<uint32_t>(offset), trianglesCount, bounds);
                return node;
            } else {
                if (trianglesCount <= 2u) {
                    auto mid = (start + end) / 2;
                    std::nth_element(&trianglesInfo[start], &trianglesInfo[mid], &trianglesInfo[end - 1] + 1,
                                     [splitAxis](const BVHTriangleInfo &a, const BVHTriangleInfo &b) {
                                         return a.bounds.centroid()[splitAxis] < b.bounds.centroid()[splitAxis];
                                     });
                } else {
                    constexpr uint8_t bucketsCount = 12u;
                    BucketInfo buckets[bucketsCount]{};

                    for (auto i = start; i < end; i++) {
                        auto b = bucketsCount * static_cast<uint32_t>(centroidBounds.offset(trianglesInfo[i].bounds.centroid())[splitAxis]);
                        if (b == bucketsCount) {
                            b = bucketsCount - 1u;
                        }

                        buckets[b].count++;
                        buckets[b].bounds.grow(trianglesInfo[i].bounds);
                    }

                    float cost[bucketsCount - 1];
                    for (auto i = 0u; i < bucketsCount - 1u; i++) {
                        NOX::BoundingBox b0{}, b1{};
                        auto count0 = 0u, count1 = 0u;

                        for (auto j = 0u; j <= i; j++) {
                            b0.grow(buckets[j].bounds);
                            count0 += buckets[j].count;
                        }

                        for (auto j = i + 1u; j < bucketsCount; j++) {
                            b1.grow(buckets[j].bounds);
                            count1 += buckets[j].count;
                        }

                        cost[i] = 1 + (count0 * b0.surfaceArea() + count1 * b1.surfaceArea()) / bounds.surfaceArea();
                    }

                    float minCost = cost[0];
                    auto minCostSplitBucket = 0u;
                    for (auto i = 1u; i < bucketsCount - 1u; i++) {
                        if (cost[i] < minCost) {
                            minCost = cost[i];
                            minCostSplitBucket = i;
                        }
                    }

                    constexpr uint32_t maxPrimsInNode = 4u;
                    float leafCost = static_cast<float>(trianglesCount);
                    if (trianglesCount > maxPrimsInNode || minCost < leafCost) {
                        auto *pMid = std::partition(&trianglesInfo[start], &trianglesInfo[end - 1] + 1,
                                                    [=](const BVHTriangleInfo &pi) {
                                                        auto b = static_cast<uint32_t>(bucketsCount * centroidBounds.offset(pi.bounds.centroid())[splitAxis]);
                                                        if (b == bucketsCount) {
                                                            b = bucketsCount - 1;
                                                        }

                                                        return b <= minCostSplitBucket;
                                                    });
                        mid = static_cast<uint32_t>(pMid - &trianglesInfo[0]);
                    } else {
                        auto offset = m_orderedTriangles.size();
                        for (auto i = start; i < end; i++) {
                            m_orderedTriangles.push_back(triangles[trianglesInfo[i].index]);
                        }

                        node->initLeaf(static_cast<uint32_t>(offset), trianglesCount, bounds);
                        return node;
                    }
                }

                node->initNode(splitAxis, subdivide(triangles, trianglesInfo, start, mid, totalNodes), subdivide(triangles, trianglesInfo, mid, end, totalNodes));
            }
        }

        return node;
    }

    uint32_t BVH::buildNodesBuffer(BVHNode *node, uint32_t &offset) {
        auto &newNode = m_nodes[offset];
        const auto &boundingBoxMinimum = node->bounds.minimum();
        const auto &boundingBoxMaximum = node->bounds.maximum();
        newNode.bounds = {{boundingBoxMinimum.x, boundingBoxMinimum.y, boundingBoxMinimum.z},
                          {boundingBoxMaximum.x, boundingBoxMaximum.y, boundingBoxMaximum.z}};

        auto tempOffset = offset++;
        if (node->triangleCount > 0u) {
            newNode.triangleCount = node->triangleCount;
            newNode.firstTriangleOffset = node->firstTriangleOffset;
        } else {
            newNode.splitAxis = node->splitAxis;
            newNode.triangleCount = 0u;
            buildNodesBuffer(node->leftChild, offset);
            newNode.firstTriangleOffset = buildNodesBuffer(node->rightChild, offset);
        }

        return tempOffset;
    }

    void BVH::cleanupNodes(BVHNode *node) {
        if (node == nullptr) {
            return;
        }

        auto leftChild = node->leftChild;
        auto rightChild = node->rightChild;
        delete node;

        cleanupNodes(leftChild);
        cleanupNodes(rightChild);
    }

} // namespace NOXPT

#pragma once

#include "kernel_types.h"

#include <vector>

namespace NOXPT {

    struct BVHNode;
    struct BVHTriangleInfo;

    class BVH {
      public:
        const std::vector<KernelTypes::BVHNode> &getBvhNodes() const { return m_nodes; }
        const std::vector<KernelTypes::Triangle> &getOrderedTriangles() const { return m_orderedTriangles; }

        void build(const std::vector<KernelTypes::Triangle> &triangles);

      private:
        BVHNode *subdivide(const std::vector<KernelTypes::Triangle> &triangles, std::vector<BVHTriangleInfo> &trianglesInfo, const uint32_t start, const uint32_t end, uint32_t &totalNodes);
        uint32_t buildNodesBuffer(BVHNode *node, uint32_t &offset);
        void cleanupNodes(BVHNode *node);

      private:
        std::vector<KernelTypes::BVHNode> m_nodes{};
        std::vector<KernelTypes::Triangle> m_orderedTriangles{};
    };

} // namespace NOXPT

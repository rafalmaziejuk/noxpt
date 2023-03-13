#ifndef BVH_NODE_H_
#define BVH_NODE_H_

#include "include/bounding_box.h"

typedef struct {
    BoundingBox bounds;
    uint firstTriangleOffset;
    uint triangleCount;
    uint splitAxis;
    uint padding;
} BVHNode;

#endif

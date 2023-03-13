#pragma once

#include <CL/cl.h>

namespace NOXPT::KernelTypes {

    struct Light {
        cl_float3 position;
        cl_float3 emission;
        cl_float3 u;
        cl_float3 v;
        cl_float area;
        cl_uint padding[3];
    };

    struct Material {
        cl_float3 diffuse;
        cl_float3 emissive;
        cl_uint padding[4];
    };

    struct Vertex {
        cl_float3 position;
        cl_float3 normal;
    };

    struct Triangle {
        Vertex v0;
        Vertex v1;
        Vertex v2;
        cl_uint materialIndex;
        cl_uint padding[3];
    };

    struct BoundingBox {
        cl_float3 minimum;
        cl_float3 maximum;
    };

    struct BVHNode {
        BoundingBox bounds;
        cl_uint firstTriangleOffset;
        cl_uint triangleCount;
        cl_uint splitAxis;
        cl_uint padding;
    };

} // namespace NOXPT::KernelTypes

#ifndef RAY_H_
#define RAY_H_

#include "include/bvh_node.h"
#include "include/hit.h"
#include "include/light.h"
#include "include/plane.h"
#include "include/triangle.h"

typedef struct {
    float3 origin;
    float3 direction;
} Ray;

bool intersect_ray_triangle(const float3 O, const float3 D, const Triangle *triangle, Hit *hit) {
    const float3 V0 = triangle->v0.position;
    const float3 V1 = triangle->v1.position;
    const float3 V2 = triangle->v2.position;
    
    const float3 E1 = V1 - V0;
    const float3 E2 = V2 - V0;
    const float3 T = O - V0;
    const float3 P = cross(D, E2);
    const float3 Q = cross(T, E1);
    const float determinant = dot(P, E1);
    const float invertedDeterminant = 1.0f / determinant;

    if (determinant < FLT_EPSILON) {
        return false;
    }
 
    const float u = invertedDeterminant * dot(P, T);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    
    const float v = invertedDeterminant * dot(Q, D);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
 
    const float t = invertedDeterminant * dot(Q, E2);
    if (t < 0.0f || t > hit->tNearest) {
        return false;
    }
    
    hit->tNearest = t;
    hit->u = u;
    hit->v = v;
 
    return true; 
}

bool intersect_ray_plane(const float3 origin, const float3 direction, const Plane *plane, Hit *hit) {
    const float denominator = dot(direction, plane->normal);
    const float t = (dot(plane->normal, plane->position) - dot(plane->normal, origin)) / denominator;

    if (fabs(t) > FLT_EPSILON) {
        float3 intersectionPoint = origin + direction * t;
        float3 vi = intersectionPoint - plane->position;
        const float a1 = dot(plane->u, vi);

        if (a1 >= 0.0f && a1 <= 1.0f) {
            const float a2 = dot(plane->v, vi);

            if ((a2 >= 0.0f && a2 <= 1.0f) && (t < hit->tNearest)) {
                hit->tNearest = t;
                return true;
            }
        }
    }

    return false;
}

bool intersect_ray_bounding_box(const float3 origin, const float3 direction, const float tNearest, const BoundingBox *bounds) {
    float3 t0 = (float3)((bounds->minimum.x - origin.x) / direction.x, (bounds->minimum.y - origin.y) / direction.y, (bounds->minimum.z - origin.z) / direction.z);
    float3 t1 = (float3)((bounds->maximum.x - origin.x) / direction.x, (bounds->maximum.y - origin.y) / direction.y, (bounds->maximum.z - origin.z) / direction.z);

    float tMin = max(max(min(t0.x, t1.x), min(t0.y, t1.y)), min(t0.z, t1.z));
    float tMax = min(min(max(t0.x, t1.x), max(t0.y, t1.y)), max(t0.z, t1.z));

    return (tMax >= tMin) && (tMin < tNearest) && (tMax > 0.0f);
}

bool intersect_ray_light(const Ray *ray, const Light *light, const Hit *hit) {
    Hit lightHit;
    lightHit.tNearest = hit->tNearest;

    Plane plane;
    plane.position = light->position;
    plane.normal = normalize(cross(light->u, light->v));
    plane.u = light->u * (1.0f / dot(light->u, light->u));
    plane.v = light->v * (1.0f / dot(light->v, light->v));

    if (dot(plane.normal, ray->direction) > 0.0f) {
        return false;
    }

    if (intersect_ray_plane(ray->origin, ray->direction, &plane, &lightHit)) {
        if (lightHit.tNearest > 0.0f) {
            return true;
        }
    }

    return false;
}

Hit intersect_ray_bvh(const Ray *ray, const BVHNode *nodes, const Triangle *triangles) {
    Hit hit;
    hit.tNearest = FLT_MAX;
    hit.isHit = false;

    uint currentNodeIndex = 0u;
    uint nodesToVisit[64];
    uint offsetToVisit = 0u;
    float3 invertedDirection = 1.0f / ray->direction;
    bool isDirectionNegative[3] = {invertedDirection.x < 0.0f, invertedDirection.y < 0.0f, invertedDirection.z < 0.0f};

    while (true) {
        const BVHNode *currentNode = &nodes[currentNodeIndex];

        if (intersect_ray_bounding_box(ray->origin, ray->direction, hit.tNearest, &currentNode->bounds)) {
            if (currentNode->triangleCount > 0u) {
                for (uint i = 0u; i < currentNode->triangleCount; i++) {
                    if (intersect_ray_triangle(ray->origin, ray->direction, &triangles[currentNode->firstTriangleOffset + i], &hit)) {
                        hit.triangleIndex = currentNode->firstTriangleOffset + i;
                        hit.isHit = true;
                    }
                }

                if (offsetToVisit == 0u) {
                    break;
                }

                currentNodeIndex = nodesToVisit[--offsetToVisit];
            } else {
                if (isDirectionNegative[currentNode->splitAxis]) {
                    nodesToVisit[offsetToVisit++] = currentNodeIndex + 1;
                    currentNodeIndex = currentNode->firstTriangleOffset;
                } else {
                    nodesToVisit[offsetToVisit++] = currentNode->firstTriangleOffset;
                    currentNodeIndex++;
                }
            }
        } else {
            if (offsetToVisit == 0u) {
                break;
            }

            currentNodeIndex = nodesToVisit[--offsetToVisit];
        }
    }

    return hit;
}

#endif

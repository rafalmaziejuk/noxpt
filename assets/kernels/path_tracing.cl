#include "include/lambert.h"
#include "include/light.h"
#include "include/material.h"
#include "include/ray.h"
#include "include/sampling.h"
#include "include/utilities.h"

__kernel void generate_primary_ray(const float3 position,
                                   const float3 forward,
                                   const float3 right,
                                   const float3 up,
                                   const float fov,
                                   const float width,
                                   const float height,
                                   const float aspectRatio,
                                   const uint sampleCount,
                                   __global Ray *rays) {
    const uint x = get_global_id(0);
    const uint y = get_global_id(1);
    const uint index = x + y * (uint)(width);

    uint2 seed = (uint2)(x, y) ^ (uint2)(sampleCount << 16u);
    const float2 random = 2.0f * random2f(&seed);

    const float2 jitter = (random < 1.0f) ? (sqrt(random) - 1.0f) : (1.0f - sqrt(2.0f - random));
    float pixelScreenX = (2.0f * (((float)(x) + jitter.x) / width) - 1.0f) * aspectRatio * fov;
    float pixelScreenY = (2.0f * (((float)(y) + jitter.y) / height) - 1.0f) * fov;

    Ray primaryRay;
    primaryRay.origin = position;
    primaryRay.direction = normalize(pixelScreenX * right + pixelScreenY * up + forward);
    rays[index] = primaryRay;
}

__kernel void trace_path(__global Ray *rays,
                        __global const BVHNode *bvhNodes,
                        __global const Triangle *triangles,
                        __global const Light *lights,
                        const uint lightsCount,
                        const uint maxBounces,
                        const uint sampleCount,
                        const uint width,
                        __global const Material *materials,
                        __global float3 *radiance) {
    const uint x = get_global_id(0);
    const uint y = get_global_id(1);
    const uint index = x + y * (uint)(width);
    uint2 seed = (uint2)(x, y) ^ (uint2)(sampleCount << 16u);

    Ray *ray = &rays[index];
    float3 throughput = 1.0f;
    for (uint bounce = 0u; bounce <= maxBounces; bounce++) {
        Hit hit = intersect_ray_bvh(ray, bvhNodes, triangles);

        if (!hit.isHit) {
            break;
        }

        const Triangle *triangle = &triangles[hit.triangleIndex];
        const float3 intersectionPoint = ray->origin + hit.tNearest * ray->direction;
        const float3 normal = interpolate3(triangle->v0.normal, triangle->v1.normal, triangle->v2.normal, hit.u, hit.v);
        const Material *material = &materials[triangle->materialIndex];

        radiance[index] += (material->emissive * throughput);

        const Light *light = &lights[0];
        if (intersect_ray_light(ray, light, &hit) && (bounce == 0u)) {
            radiance[index] += (light->emission * throughput);
            break;
        }

        LightSample lightSample = sample_rectangle_light(light, intersectionPoint, random2f(&seed));
        if (dot(lightSample.direction, lightSample.normal) < 0.0f) {
            Ray shadowRay;
            shadowRay.origin = intersectionPoint;
            shadowRay.direction = lightSample.direction;
            Hit shadowHit = intersect_ray_bvh(&shadowRay, bvhNodes, triangles);

            if (shadowHit.tNearest >= lightSample.distance) {
                const BRDFSample brdfSample = evaluate_lambert_brdf(material, normal, lightSample.direction);
                const float3 Li = light->emission;
                const float3 Ld = (Li * brdfSample.brdf * brdfSample.cosTheta) / lightSample.pdf;

                if (brdfSample.pdf > 0.0f) {
                    radiance[index] += (Ld * throughput);
                }
            }
        }

        BRDFSample brdfSample = sample_lambert_brdf(material, normal, random2f(&seed));
        
        ray->direction = brdfSample.direction;
        ray->origin = intersectionPoint + ray->direction * FLT_EPSILON;
        
        const float3 Lr = (brdfSample.brdf * brdfSample.cosTheta) / brdfSample.pdf;
        throughput *= Lr;

        if (bounce > 3u) {
            const float throughputMax = max(throughput.x, max(throughput.y, throughput.z));
            const float q = max(0.05f, 1.0f - throughputMax);
            if (random1f(&seed) < q) {
                break;
            }
            throughput /= (1.0f - q);
        }
    }
}

__kernel void compute_pixel(__read_write image2d_t imagePlane,
                            __global const float3 *radiance,
                            const uint sampleCount) {
    const uint x = get_global_id(0);
    const uint y = get_global_id(1);
    const uint index = x + get_image_width(imagePlane) * y;

    const float samples = (float)sampleCount;
    const float3 previousRadiance = read_imagef(imagePlane, (int2)(x, y)).xyz;
    const float3 newRadiance = radiance[index] / samples;
    const float3 color = mix(newRadiance, previousRadiance, 1.0f / (samples + 1.0f));
    const float3 gammaCorrectedColor = gamma_correction(color);
    const float3 toneMappedColor = tone_mapping(gammaCorrectedColor);

    write_imagef(imagePlane, (int2)(x, y), (float4)(toneMappedColor, 1.0f));
}

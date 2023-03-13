#ifndef SAMPLING_H_
#define SAMPLING_H_

#include "include/utilities.h"

float3 uniform_sample_hemisphere(const float3 normal, const float2 random) {
    const float phi = 2 * M_PI_F * random.y;
    const float r = sqrt(max(0.0f, 1.0f - random.x * random.x));

    const float x = cos(phi) * r;
    const float y = sin(phi) * r;
    const float z = random.x;

    return transform_to_world(x, y, z, normal);
}

float uniform_hemisphere_pdf(void) {
    return 1.0f / (2.0f * M_PI_F);
}

float3 cosine_weighted_sample_hemisphere(const float3 normal, const float2 random) {
    const float phi = 2.0f * M_PI_F * random.y;
    const float r = sqrt(max(0.0f, random.x));

    const float x = cos(phi) * r;
    const float y = sin(phi) * r;
    const float z = sqrt(max(0.0f, 1.0f - random.x));

    return transform_to_world(x, y, z, normal);
}

float cosine_weighted_hemisphere_pdf(const float cosTheta) {
    return cosTheta * M_1_PI_F;
}

#endif

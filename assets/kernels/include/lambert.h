#ifndef LAMBERT_H_
#define LAMBERT_H_

#include "include/brdf_sample.h"
#include "include/material.h"
#include "include/sampling.h"

float3 lambert_brdf(const Material *material) {
    return material->diffuse * M_1_PI_F;
}

BRDFSample evaluate_lambert_brdf(const Material *material, const float3 normal, const float3 direction) {
    BRDFSample result;
    result.brdf = lambert_brdf(material);
    result.cosTheta = dot(direction, normal);
    result.pdf = cosine_weighted_hemisphere_pdf(result.cosTheta);

    return result;
}

BRDFSample sample_lambert_brdf(const Material *material, const float3 normal, const float2 random) {
    BRDFSample result;
    result.direction = cosine_weighted_sample_hemisphere(normal, random);
    result.brdf = lambert_brdf(material);
    result.cosTheta = dot(result.direction, normal);
    result.pdf = cosine_weighted_hemisphere_pdf(result.cosTheta);
    
    return result;
}

#endif

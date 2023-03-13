#ifndef BRDF_SAMPLE_H_
#define BRDF_SAMPLE_H_

typedef struct {
    float3 direction;
    float3 brdf;
    float cosTheta;
    float pdf;
} BRDFSample;

#endif

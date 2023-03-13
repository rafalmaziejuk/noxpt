#ifndef MATERIAL_H_
#define MATERIAL_H_

typedef struct {
    float3 diffuse;
    float3 emissive;
    uint padding[4];
} Material;

#endif

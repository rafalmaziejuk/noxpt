#ifndef LIGHT_H_
#define LIGHT_H_

typedef struct {
    float3 position;
    float3 emission;
    float3 u;
    float3 v;
    float area;
    uint padding[3];
} Light;

typedef struct {
    float3 normal;
    float3 emission;
    float3 direction;
    float distance;
    float pdf;
} LightSample;

LightSample sample_rectangle_light(const Light *light, const float3 intersectionPoint, const float2 random) {
    const float3 lightSurfacePosition = light->position + light->u * random.x + light->v * random.y;

    LightSample result;
    result.normal = normalize(cross(light->u, light->v));
    result.emission = light->emission;
    result.direction = lightSurfacePosition - intersectionPoint;
    result.distance = length(result.direction);
    result.direction /= result.distance;

    const float distanceSquared = result.distance * result.distance;
    result.pdf = distanceSquared / (light->area * fabs(dot(result.normal, result.direction)));

    return result;
}

#endif

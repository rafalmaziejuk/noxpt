#ifndef UTILITIES_H_
#define UTILITIES_H_

#define RANDOM_CONSTANT 2.32830643654e-10f
#define TONE_MAPPING_LIMIT 1.5f

float3 interpolate3(const float3 a, const float3 b, const float3 c, const float u, const float v) {
    return (1.0f - u - v) * a + u * b + v * c;
}

void prng(uint2 *seed) {
    *seed = 1664525u * *seed + 1013904223u;
    seed->x += 1664525u * seed->y;
    seed->y += 1664525u * seed->x;
    *seed ^= (*seed >> 16u);
    seed->x += 1664525u * seed->y;
    seed->y += 1664525u * seed->x;
    *seed ^= (*seed >> 16u);
}

float random1f(uint2 *seed) {
    prng(seed);
    return (float)seed->x * RANDOM_CONSTANT;
}

float2 random2f(uint2 *seed) {
    prng(seed);
    return (float2)(seed->x, seed->y) * RANDOM_CONSTANT;
}

float3 transform_to_world(const float x, const float y, const float z, const float3 normal) {
    const float3 u = normalize(cross(normal, (float3)(0.0f, 1.0f, 1.0f)));
    const float3 v = cross(normal, u);
    const float3 w = normal;

    return normalize(u * x + v * y + w * z);
}

float3 gamma_correction(const float3 color) {
    return pow(color, (float3)(1.0f / 2.2f));
}

float luminance(const float3 color) {
    return 0.212671f * color.x + 0.715160f * color.y + 0.072169f * color.z;
}

float3 tone_mapping(const float3 color) {
    return color * (1.0f / (1.0f + (luminance(color) / TONE_MAPPING_LIMIT)));
}

#endif

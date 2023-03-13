#ifndef HIT_H_
#define HIT_H_

typedef struct {
    float tNearest;
    float u, v;
    uint triangleIndex;
    bool isHit;
} Hit;

#endif

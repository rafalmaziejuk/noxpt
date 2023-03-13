#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "include/vertex.h"

typedef struct {
    Vertex v0;
    Vertex v1;
    Vertex v2;
    uint materialIndex;
    uint padding[3];
} Triangle;

#endif

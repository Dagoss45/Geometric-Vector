#ifndef  GEOMETRIC_VECTOR_H
#define GEOMETRIC_VECTOR_H

#include <stdlib.h>
#include <math.h>

typedef struct
{
    int dim;
    float coordinates[];
} Vector;

Vector *create_vector(const float coordinates[], const int *dim);
void destroy_vector(Vector **v);
Vector *algebric_sum(const Vector *v1, const Vector *v2, const int *operation);
float dot_product(const Vector *v1, const Vector *v2);
Vector *cross_product(const Vector *v1, const Vector *v2);
float norm(const Vector *v);

#endif

#include "geometric_vector.h"
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

Vector *create_vector(const float coordinates[], const int *dim) {
    if(coordinates == NULL || dim == NULL) return NULL;
    if((*dim) <= 0) return NULL;

    // Check for multiplication overflow before allocation
    if ((size_t)(*dim) > (SIZE_MAX - sizeof(Vector)) / sizeof(float))
        return NULL;

    Vector *v = malloc(sizeof(Vector) + sizeof(float) * (*dim));
    if(v == NULL) return NULL;

    v->dim = (*dim);
    for(int i = 0; i < (*dim); i++) {
        v->coordinates[i] = coordinates[i];
    }
    return v;
}

/* Frees the memory allocated for the vector and sets the original pointer to NULL */
void destroy_vector(Vector **v) {
    if(v != NULL && *v != NULL) {
        free(*v);
        *v = NULL;
    }
}

/* the opeation parameter specifies whether the operation is addition (number 0) */
/* or subtraction (number 1). */
Vector *algebric_sum(const Vector *v1, const Vector *v2, const int *operation) {
    if(v1 == NULL || v2 == NULL) return NULL;
    if(v1->dim != v2->dim) return NULL;
    if(operation == NULL) return NULL;

    int size = v1->dim;
    // Direct dynamic allocation, avoiding VLA on stack
    Vector *result = malloc(sizeof(Vector) + sizeof(float) * size);
    if(result == NULL) return NULL;
    result->dim = size;

    if((*operation) == 0) {
        for(int i = 0; i < size; i++) {
            result->coordinates[i] = v1->coordinates[i] + v2->coordinates[i];
        }
    } else if((*operation) == 1) {
        for(int i = 0; i < size; i++) {
            result->coordinates[i] = v1->coordinates[i] - v2->coordinates[i];
        }   
    }
    else {
        free(result);
        return NULL;
    }
    return result;
}

/* using the NAN macro, to later check the error it's needed the isnan() function, */
/* from the math.h library */
float dot_product(const Vector *v1, const Vector *v2) {
    if(v1 == NULL || v2 == NULL) return NAN;
    if(v1->dim != v2->dim) return NAN;

    float result = 0.0f;
    int size = v1->dim;
    for(int i = 0; i < size; i++) {
        result += (v1->coordinates[i] * v2->coordinates[i]);
    }
    return result;
}

Vector *cross_product(const Vector *v1, const Vector *v2) {
    if(v1 == NULL || v2 == NULL) return NULL;
    if(v1->dim != 3 || v2->dim != 3) return NULL;

    float coordinates[3];
    coordinates[0] = (v1->coordinates[1] * v2->coordinates[2]) - (v1->coordinates[2] * v2->coordinates[1]);
    coordinates[1] = (v1->coordinates[2] * v2->coordinates[0]) - (v1->coordinates[0] * v2->coordinates[2]);
    coordinates[2] = (v1->coordinates[0] * v2->coordinates[1]) - (v1->coordinates[1] * v2->coordinates[0]);

    // Direct allocation to avoid passing &v1->dim as size
    Vector *result = malloc(sizeof(Vector) + sizeof(float) * 3);
    if(result == NULL) return NULL;
    result->dim = 3;
    for(int i = 0; i < 3; i++) {
        result->coordinates[i] = coordinates[i];
    }
    return result;
}

float norm(const Vector *v) {
    if(v == NULL) return NAN;
    
    int size = v->dim;
    float sum = 0.0f;   
    for(int i = 0; i < size; i++) {
        float c = v->coordinates[i];
        sum += c * c;   
    }
    return sqrtf(sum);  // square root for float
}

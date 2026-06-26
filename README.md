# geometric-vector

A small, self-contained **C library** for working with geometric vectors of **arbitrary dimension** (N-D). It provides vector creation/destruction, addition/subtraction, dot product, cross product (3D), and Euclidean norm, with dynamic memory allocation and basic error handling via `NULL` / `NaN` sentinels.

---

## Table of contents

- [Features](#features)
- [Requirements](#requirements)
- [The `Vector` data structure](#the-vector-data-structure)
- [Building the library](#building-the-library)
  - [Quick build with gcc](#quick-build-with-gcc)
  - [Building a static library](#building-a-static-library)
  - [Suggested Makefile](#suggested-makefile)
- [API reference](#api-reference)
  - [`create_vector`](#create_vector)
  - [`destroy_vector`](#destroy_vector)
  - [`algebric_sum`](#algebric_sum)
  - [`dot_product`](#dot_product)
  - [`cross_product`](#cross_product)
  - [`norm`](#norm)
- [Usage example](#usage-example)
- [Error handling conventions](#error-handling-conventions)
- [Memory management](#memory-management)
- [Known issues](#known-issues)
- [Possible improvements](#possible-improvements)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- Vectors of **any positive dimension**, allocated dynamically as a single contiguous heap block.
- Element-wise **addition** and **subtraction** between two vectors of equal dimension.
- **Dot product** for vectors of any (matching) dimension.
- **Cross product**, restricted to 3-dimensional vectors as per its mathematical definition.
- **Euclidean norm** (vector length).
- Defensive checks against `NULL` inputs, dimension mismatches, and integer overflow during allocation.
- No external dependencies beyond the C standard library (`stdlib.h`, `math.h`, `stdint.h`).

## Requirements

- A C compiler supporting **C99 or later** (the `Vector` struct uses a flexible array member, a C99 feature).
- The math library (`libm`) for linking, since `sqrtf` is used internally.
- Tested with GCC and Clang on Linux; should work unmodified on any standard-compliant C99/C11 toolchain.

## The `Vector` data structure

```c
typedef struct
{
    int dim;               // number of dimensions
    float coordinates[];   // flexible array member (C99+)
} Vector;
```

`coordinates` is a **flexible array member (FAM)**: it doesn't occupy space inside the struct itself, but is laid out in memory immediately after `dim`, as part of the *same* heap allocation. This keeps the dimension count and the coordinate data contiguous, avoiding a second allocation:

```
+--------+-----------------------------------------+
|  dim   |  coords[0]   coords[1]   ...  coords[N-1]|
+--------+-----------------------------------------+
  4 bytes              dim * 4 bytes
```

Because of the FAM, `Vector` instances **must** be created with `create_vector` (which performs the correctly-sized single `malloc`) and **must never** be declared as plain stack variables.

---

## Building the library

The library consists of two files:

- `geometric_vector.h` — public header (struct + function declarations)
- `geometric_vector.c` — implementation

### Quick build with gcc

To compile the library together with your own program (e.g. `main.c`) into a single executable:

```bash
gcc -std=c11 -Wall -Wextra -o my_program geometric_vector.c main.c -lm
```

To compile the library into an object file you can link later:

```bash
gcc -std=c11 -Wall -Wextra -c geometric_vector.c -o geometric_vector.o
gcc -std=c11 -Wall -Wextra main.c geometric_vector.o -o my_program -lm
```

> `-lm` is required because the library calls `sqrtf` from `<math.h>`.

### Building a static library

If you want to reuse `geometric_vector` across multiple projects, build it as a static library:

```bash
gcc -std=c11 -Wall -Wextra -c geometric_vector.c -o geometric_vector.o
ar rcs libgeometricvector.a geometric_vector.o
```

Then link against it:

```bash
gcc -std=c11 -Wall -Wextra main.c -L. -lgeometricvector -lm -o my_program
```

### Suggested Makefile

```makefile
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
LDLIBS  = -lm

all: example

example: geometric_vector.o example.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c geometric_vector.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o example
```

---

## API reference

### `create_vector`

```c
Vector *create_vector(const float coordinates[], const int *dim);
```

Allocates a new `Vector` and copies `*dim` coordinates into it.

| Parameter | Description |
|---|---|
| `coordinates` | Array of `float` values, must contain at least `*dim` elements |
| `dim` | Pointer to the number of dimensions (must be `> 0`) |

**Returns:** a pointer to the newly allocated `Vector`, or `NULL` if:
- `coordinates` or `dim` is `NULL`,
- `*dim` is less than or equal to `0`,
- the requested size would overflow during allocation,
- the underlying `malloc` fails.

**Complexity:** O(n)

```c
float coords[] = {1.0f, 2.0f, 3.0f};
int dim = 3;
Vector *v = create_vector(coords, &dim);
```

### `destroy_vector`

```c
void destroy_vector(Vector **v);
```

Frees the memory owned by a `Vector` and sets the caller's pointer to `NULL`, to avoid dangling-pointer reuse.

| Parameter | Description |
|---|---|
| `v` | Address of the vector pointer to destroy |

**Returns:** nothing. Safe to call with `v == NULL` or `*v == NULL` (no-op in both cases).

```c
Vector *v = create_vector(coords, &dim);
/* ... use v ... */
destroy_vector(&v); // v is now NULL
```

### `algebric_sum`

```c
Vector *algebric_sum(const Vector *v1, const Vector *v2, const int *operation);
```

Computes the element-wise sum or difference of two vectors of equal dimension.

| Parameter | Description |
|---|---|
| `v1`, `v2` | Operand vectors, must share the same `dim` |
| `operation` | Pointer to an `int`: `0` for addition, `1` for subtraction |

**Returns:** a newly allocated `Vector` with the result, or `NULL` if:
- `v1`, `v2`, or `operation` is `NULL`,
- `v1->dim != v2->dim`,
- `*operation` is neither `0` nor `1`,
- allocation fails.

**Complexity:** O(n)

```c
int op = 0; // 0 = addition, 1 = subtraction
Vector *sum = algebric_sum(v1, v2, &op);
```

### `dot_product`

```c
float dot_product(const Vector *v1, const Vector *v2);
```

Computes the scalar (dot) product of two vectors of equal dimension.

| Parameter | Description |
|---|---|
| `v1`, `v2` | Operand vectors, must share the same `dim` |

**Returns:** the dot product as a `float`, or `NAN` if `v1`/`v2` is `NULL` or their dimensions differ.

**Complexity:** O(n)

```c
float result = dot_product(v1, v2);
if (isnan(result)) {
    /* invalid input */
}
```

> Since `NaN != NaN` under IEEE 754, never compare the result with `== NAN`; always use `isnan()` from `<math.h>`.

### `cross_product`

```c
Vector *cross_product(const Vector *v1, const Vector *v2);
```

Computes the 3-dimensional cross product of two vectors.

| Parameter | Description |
|---|---|
| `v1`, `v2` | Operand vectors, **both must have `dim == 3`** |

**Returns:** a newly allocated 3D `Vector` with the result, or `NULL` if `v1`/`v2` is `NULL` or either does not have exactly 3 dimensions.

**Complexity:** O(1)

```c
Vector *result = cross_product(v1, v2); // v1 and v2 must both be 3D
```

### `norm`

```c
float norm(const Vector *v);
```

Computes the Euclidean norm (length) of a vector.

| Parameter | Description |
|---|---|
| `v` | The vector to measure |

**Returns:** the norm as a `float`, or `NAN` if `v` is `NULL`.

**Complexity:** O(n)

```c
float length = norm(v);
```

---

## Usage example

```c
#include <stdio.h>
#include <math.h>
#include "geometric_vector.h"

int main(void) {
    float coords_a[] = {1.0f, 2.0f, 3.0f};
    float coords_b[] = {4.0f, 5.0f, 6.0f};
    int dim = 3;

    Vector *a = create_vector(coords_a, &dim);
    Vector *b = create_vector(coords_b, &dim);
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Failed to create vectors\n");
        return 1;
    }

    int op_add = 0;
    Vector *sum = algebric_sum(a, b, &op_add);

    float dp = dot_product(a, b);
    Vector *cp = cross_product(a, b);
    float na = norm(a);

    printf("a       = (%.2f, %.2f, %.2f)\n", a->coordinates[0], a->coordinates[1], a->coordinates[2]);
    printf("b       = (%.2f, %.2f, %.2f)\n", b->coordinates[0], b->coordinates[1], b->coordinates[2]);
    if (sum) printf("a + b   = (%.2f, %.2f, %.2f)\n", sum->coordinates[0], sum->coordinates[1], sum->coordinates[2]);
    printf("a . b   = %.2f\n", dp);
    if (cp) printf("a x b   = (%.2f, %.2f, %.2f)\n", cp->coordinates[0], cp->coordinates[1], cp->coordinates[2]);
    printf("||a||   = %.4f\n", na);

    destroy_vector(&a);
    destroy_vector(&b);
    destroy_vector(&sum);
    destroy_vector(&cp);

    return 0;
}
```

Compile and run it:

```bash
gcc -std=c11 -Wall -Wextra -o example geometric_vector.c example.c -lm
./example
```

Expected output:

```
a       = (1.00, 2.00, 3.00)
b       = (4.00, 5.00, 6.00)
a + b   = (5.00, 7.00, 9.00)
a . b   = 32.00
a x b   = (-3.00, 6.00, -3.00)
||a||   = 3.7417
```

---

## Error handling conventions

- Functions returning `Vector *` signal an error by returning `NULL`. **Always check the return value before dereferencing it.**
- Functions returning `float` (`dot_product`, `norm`) signal an error by returning `NAN`. Check with `isnan()` from `<math.h>`, never with `==`.
- No function sets `errno`; all errors are communicated through return values only.

## Memory management

- Every successful call to `create_vector`, `algebric_sum`, or `cross_product` allocates memory on the heap that **you own** and must release with `destroy_vector`.
- `destroy_vector` takes the *address* of the pointer (`Vector **`) and nulls it out after freeing, which makes accidental double-free or use-after-free easier to avoid — calling `destroy_vector` again on the same variable is always safe.
- `dot_product` and `norm` never allocate memory and require no cleanup.
- The library performs no allocation tracking; it's the caller's responsibility to destroy every vector it creates.

---

## Known issues

No correctness, memory-safety, or undefined-behavior issues are currently known in this version: the code compiles cleanly with `-std=c11 -Wall -Wextra -Wpedantic`, all `NULL` checks are in place, the integer-overflow guard in `create_vector` is correct, dynamic allocation (rather than a stack VLA) is used throughout, and `cross_product` has been verified against known reference values (e.g. `(1,2,3) × (4,5,6) = (-3, 6, -3)`).

If you find a bug, please open an issue with a minimal reproducible example.

## Possible improvements

- [ ] Add a `print_vector` helper for debugging.
- [ ] Add a `normalize` function returning a unit vector.
- [ ] Add a unit test suite (e.g. with Unity or CMocka) and wire it into the Makefile.
- [ ] Consider passing `dim`/`operation` by value instead of by pointer for a simpler API, since there's no functional need for indirection there.
- [ ] Add CI (GitHub Actions) to build and test on every push.

## Contributing

Contributions are welcome!

1. Fork the repository.
2. Create a branch for your change (`git checkout -b feature/normalize-vector`).
3. Make sure the code builds warning-free with `-Wall -Wextra -Wpedantic`.
4. Open a Pull Request describing the change and, if fixing a bug, how to reproduce it.

When reporting a bug, please include your compiler/version, OS, and a minimal reproducible example.

## License

This project is licensed under the **MIT License** — see the [`LICENSE`](LICENSE) file for the full text. In short: you are free to use, copy, modify, merge, publish, distribute, and sublicense this software, including for commercial purposes, as long as the original copyright notice and license text are included with any substantial portion of the software. The software is provided "as is", without warranty of any kind.

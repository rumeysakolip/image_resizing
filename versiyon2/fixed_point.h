#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>

// Fixed-point precision (Q16.16 format)
#define FIXED_SHIFT 16
#define FIXED_ONE (1 << FIXED_SHIFT)
#define FIXED_HALF (1 << (FIXED_SHIFT - 1))

// Convert float to fixed-point
static inline int32_t float_to_fixed(float value) {
    return (int32_t)(value * FIXED_ONE);
}

// Convert fixed-point to float
static inline float fixed_to_float(int32_t value) {
    return (float)value / FIXED_ONE;
}

// Fixed-point multiplication
static inline int32_t fixed_mult(int32_t a, int32_t b) {
    return (int32_t)(((int64_t)a * b) >> FIXED_SHIFT);
}

// Fixed-point division
static inline int32_t fixed_div(int32_t a, int32_t b) {
    return (int32_t)(((int64_t)a << FIXED_SHIFT) / b);
}

// Extract integer part from fixed-point
static inline int32_t fixed_int_part(int32_t value) {
    return value >> FIXED_SHIFT;
}

// Extract fractional part from fixed-point
static inline int32_t fixed_frac_part(int32_t value) {
    return value & ((1 << FIXED_SHIFT) - 1);
}

#endif // FIXED_POINT_H

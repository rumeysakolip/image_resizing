#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

#include <stdint.h>

// Fixed-point precision (Q16.16 format)
#define FIXED_SHIFT 16
#define FIXED_ONE (1 << FIXED_SHIFT)

// Define a structure for RGB pixels
typedef struct {
    uint8_t r, g, b;
} PixelRGB;

// Define a structure for grayscale pixels
typedef uint8_t PixelGray;

// Define an image structure that can handle both RGB and grayscale
typedef struct {
    void* data;         // Pointer to pixel data
    int width;          // Image width
    int height;         // Image height
    int channels;       // Number of channels (1 for grayscale, 3 for RGB)
} Image;

// Function declarations
Image* create_image(int width, int height, int channels);
void free_image(Image* img);
Image* resize_image_fixed(const Image* input, int32_t scale_num, int32_t scale_denom);
Image* resize_image_nearest(const Image* input, int32_t scale_num, int32_t scale_denom);
Image* create_test_pattern(int width, int height, int channels);
Image* load_image(const char* filename);
int save_image(const Image* img, const char* filename);

// Helper functions
uint8_t clamp_int(int value, uint8_t min, uint8_t max);

#endif // IMAGE_RESIZE_H

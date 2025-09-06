#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

#include <stdint.h>

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
Image* resize_image(const Image* input, float scale_factor);

#endif // IMAGE_RESIZE_H

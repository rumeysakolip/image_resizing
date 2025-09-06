#include "image_resize.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Create a new image with specified dimensions and type
Image* create_image(int width, int height, int channels) {
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;

    img->width = width;
    img->height = height;
    img->channels = channels;

    size_t pixel_size = channels == 3 ? sizeof(PixelRGB) : sizeof(PixelGray);
    img->data = malloc(width * height * pixel_size);

    if (!img->data) {
        free(img);
        return NULL;
    }

    return img;
}

// Free image memory
void free_image(Image* img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

// Clamp a value between min and max
static inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Bilinear interpolation for grayscale
static PixelGray bilinear_interp_gray(const Image* input, float x, float y) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Clamp coordinates to image boundaries
    x0 = clamp(x0, 0, input->width - 1);
    y0 = clamp(y0, 0, input->height - 1);
    x1 = clamp(x1, 0, input->width - 1);
    y1 = clamp(y1, 0, input->height - 1);

    float dx = x - x0;
    float dy = y - y0;

    PixelGray* data = (PixelGray*)input->data;
    PixelGray p00 = data[y0 * input->width + x0];
    PixelGray p01 = data[y0 * input->width + x1];
    PixelGray p10 = data[y1 * input->width + x0];
    PixelGray p11 = data[y1 * input->width + x1];

    // Interpolate
    float top = (1 - dx) * p00 + dx * p01;
    float bottom = (1 - dx) * p10 + dx * p11;
    float value = (1 - dy) * top + dy * bottom;

    return (PixelGray)clamp(value, 0, 255);
}

// Bilinear interpolation for RGB
static PixelRGB bilinear_interp_rgb(const Image* input, float x, float y) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Clamp coordinates to image boundaries
    x0 = clamp(x0, 0, input->width - 1);
    y0 = clamp(y0, 0, input->height - 1);
    x1 = clamp(x1, 0, input->width - 1);
    y1 = clamp(y1, 0, input->height - 1);

    float dx = x - x0;
    float dy = y - y0;

    PixelRGB* data = (PixelRGB*)input->data;
    PixelRGB p00 = data[y0 * input->width + x0];
    PixelRGB p01 = data[y0 * input->width + x1];
    PixelRGB p10 = data[y1 * input->width + x0];
    PixelRGB p11 = data[y1 * input->width + x1];

    PixelRGB result;

    // Interpolate each channel separately
    result.r = (uint8_t)clamp((1 - dy) * ((1 - dx) * p00.r + dx * p01.r) +
                             dy * ((1 - dx) * p10.r + dx * p11.r), 0, 255);
    result.g = (uint8_t)clamp((1 - dy) * ((1 - dx) * p00.g + dx * p01.g) +
                             dy * ((1 - dx) * p10.g + dx * p11.g), 0, 255);
    result.b = (uint8_t)clamp((1 - dy) * ((1 - dx) * p00.b + dx * p01.b) +
                             dy * ((1 - dx) * p10.b + dx * p11.b), 0, 255);

    return result;
}

// Main resizing function
Image* resize_image(const Image* input, float scale_factor) {
    if (!input || !input->data || scale_factor <= 0) {
        return NULL;
    }

    // Calculate output dimensions
    int out_width = (int)(input->width * scale_factor);
    int out_height = (int)(input->height * scale_factor);

    // Ensure at least 1 pixel in each dimension
    if (out_width < 1) out_width = 1;
    if (out_height < 1) out_height = 1;

    // Create output image
    Image* output = create_image(out_width, out_height, input->channels);
    if (!output) return NULL;

    // Calculate scale ratios
    float x_ratio = (float)input->width / out_width;
    float y_ratio = (float)input->height / out_height;

    // Resize the image
    for (int y = 0; y < out_height; y++) {
        for (int x = 0; x < out_width; x++) {
            // Map output pixel to input image coordinates
            float src_x = (x + 0.5f) * x_ratio - 0.5f;
            float src_y = (y + 0.5f) * y_ratio - 0.5f;

            if (input->channels == 3) {
                PixelRGB* out_data = (PixelRGB*)output->data;
                out_data[y * out_width + x] = bilinear_interp_rgb(input, src_x, src_y);
            } else {
                PixelGray* out_data = (PixelGray*)output->data;
                out_data[y * out_width + x] = bilinear_interp_gray(input, src_x, src_y);
            }
        }
    }

    return output;
}

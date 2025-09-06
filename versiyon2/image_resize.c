#include "image_resize.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Create a new image with specified dimensions and type
Image* create_image(int width, int height, int channels) {
    if (width <= 0 || height <= 0 || (channels != 1 && channels != 3)) {
        return NULL;
    }

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

    // Initialize to zero
    memset(img->data, 0, width * height * pixel_size);

    return img;
}

// Free image memory
void free_image(Image* img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

// Clamp an integer value between min and max
uint8_t clamp_int(int value, uint8_t min, uint8_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return (uint8_t)value;
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

// Integer bilinear interpolation for grayscale
static PixelGray bilinear_interp_gray_int(const Image* input, int32_t x_fixed, int32_t y_fixed) {
    int32_t x = fixed_int_part(x_fixed);
    int32_t y = fixed_int_part(y_fixed);

    // Fractional parts
    int32_t dx = fixed_frac_part(x_fixed);
    int32_t dy = fixed_frac_part(y_fixed);

    // Get four neighboring pixels (with boundary checking)
    int32_t x0 = x;
    int32_t y0 = y;
    int32_t x1 = x + 1;
    int32_t y1 = y + 1;

    // Clamp coordinates to image boundaries
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x0 >= input->width) x0 = input->width - 1;
    if (y0 >= input->height) y0 = input->height - 1;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x1 >= input->width) x1 = input->width - 1;
    if (y1 >= input->height) y1 = input->height - 1;

    PixelGray* data = (PixelGray*)input->data;
    int32_t p00 = data[y0 * input->width + x0];
    int32_t p01 = data[y0 * input->width + x1];
    int32_t p10 = data[y1 * input->width + x0];
    int32_t p11 = data[y1 * input->width + x1];

    // Interpolate using fixed-point arithmetic
    int32_t top = p00 + fixed_mult((p01 - p00), dx);
    int32_t bottom = p10 + fixed_mult((p11 - p10), dx);
    int32_t value = top + fixed_mult((bottom - top), dy);

    return clamp_int(value, 0, 255);
}

// Integer bilinear interpolation for RGB
static PixelRGB bilinear_interp_rgb_int(const Image* input, int32_t x_fixed, int32_t y_fixed) {
    int32_t x = fixed_int_part(x_fixed);
    int32_t y = fixed_int_part(y_fixed);

    // Fractional parts
    int32_t dx = fixed_frac_part(x_fixed);
    int32_t dy = fixed_frac_part(y_fixed);

    // Get four neighboring pixels (with boundary checking)
    int32_t x0 = x;
    int32_t y0 = y;
    int32_t x1 = x + 1;
    int32_t y1 = y + 1;

    // Clamp coordinates to image boundaries
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x0 >= input->width) x0 = input->width - 1;
    if (y0 >= input->height) y0 = input->height - 1;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x1 >= input->width) x1 = input->width - 1;
    if (y1 >= input->height) y1 = input->height - 1;

    PixelRGB* data = (PixelRGB*)input->data;
    PixelRGB p00 = data[y0 * input->width + x0];
    PixelRGB p01 = data[y0 * input->width + x1];
    PixelRGB p10 = data[y1 * input->width + x0];
    PixelRGB p11 = data[y1 * input->width + x1];

    PixelRGB result;

    // Interpolate each channel separately
    result.r = clamp_int(p00.r + fixed_mult((p01.r - p00.r), dx) +
                        fixed_mult((p10.r - p00.r), dy) +
                        fixed_mult((p00.r - p01.r - p10.r + p11.r), fixed_mult(dx, dy)), 0, 255);

    result.g = clamp_int(p00.g + fixed_mult((p01.g - p00.g), dx) +
                        fixed_mult((p10.g - p00.g), dy) +
                        fixed_mult((p00.g - p01.g - p10.g + p11.g), fixed_mult(dx, dy)), 0, 255);

    result.b = clamp_int(p00.b + fixed_mult((p01.b - p00.b), dx) +
                        fixed_mult((p10.b - p00.b), dy) +
                        fixed_mult((p00.b - p01.b - p10.b + p11.b), fixed_mult(dx, dy)), 0, 255);

    return result;
}

// Fixed-point image resizing with bilinear interpolation
Image* resize_image_fixed(const Image* input, int32_t scale_num, int32_t scale_denom) {
    if (!input || !input->data || scale_num <= 0 || scale_denom <= 0) {
        return NULL;
    }

    // Calculate output dimensions using integer math
    int out_width = (input->width * scale_num) / scale_denom;
    int out_height = (input->height * scale_num) / scale_denom;

    // Ensure at least 1 pixel in each dimension
    if (out_width < 1) out_width = 1;
    if (out_height < 1) out_height = 1;

    // Create output image
    Image* output = create_image(out_width, out_height, input->channels);
    if (!output) return NULL;

    // Precompute fixed-point step sizes
    int32_t x_step = fixed_div(input->width << FIXED_SHIFT, out_width << FIXED_SHIFT);
    int32_t y_step = fixed_div(input->height << FIXED_SHIFT, out_height << FIXED_SHIFT);

    // Resize the image using fixed-point coordinates
    int32_t y_src_fixed = 0;
    for (int y = 0; y < out_height; y++) {
        int32_t x_src_fixed = 0;
        for (int x = 0; x < out_width; x++) {
            if (input->channels == 3) {
                PixelRGB* out_data = (PixelRGB*)output->data;
                out_data[y * out_width + x] = bilinear_interp_rgb_int(input, x_src_fixed, y_src_fixed);
            } else {
                PixelGray* out_data = (PixelGray*)output->data;
                out_data[y * out_width + x] = bilinear_interp_gray_int(input, x_src_fixed, y_src_fixed);
            }
            x_src_fixed += x_step;
        }
        y_src_fixed += y_step;
    }

    return output;
}

// Nearest neighbor interpolation (simplest for hardware)
Image* resize_image_nearest(const Image* input, int32_t scale_num, int32_t scale_denom) {
    if (!input || !input->data || scale_num <= 0 || scale_denom <= 0) {
        return NULL;
    }

    int out_width = (input->width * scale_num) / scale_denom;
    int out_height = (input->height * scale_num) / scale_denom;

    // Ensure at least 1 pixel in each dimension
    if (out_width < 1) out_width = 1;
    if (out_height < 1) out_height = 1;

    Image* output = create_image(out_width, out_height, input->channels);
    if (!output) return NULL;

    // Precompute step sizes
    int32_t x_ratio = (input->width << FIXED_SHIFT) / out_width;
    int32_t y_ratio = (input->height << FIXED_SHIFT) / out_height;

    for (int y = 0; y < out_height; y++) {
        for (int x = 0; x < out_width; x++) {
            int32_t src_x = (x * x_ratio) >> FIXED_SHIFT;
            int32_t src_y = (y * y_ratio) >> FIXED_SHIFT;

            // Clamp coordinates
            if (src_x < 0) src_x = 0;
            if (src_y < 0) src_y = 0;
            if (src_x >= input->width) src_x = input->width - 1;
            if (src_y >= input->height) src_y = input->height - 1;

            if (input->channels == 3) {
                PixelRGB* in_data = (PixelRGB*)input->data;
                PixelRGB* out_data = (PixelRGB*)output->data;
                out_data[y * out_width + x] = in_data[src_y * input->width + src_x];
            } else {
                PixelGray* in_data = (PixelGray*)input->data;
                PixelGray* out_data = (PixelGray*)output->data;
                out_data[y * out_width + x] = in_data[src_y * input->width + src_x];
            }
        }
    }

    return output;
}

// Create a test pattern image
Image* create_test_pattern(int width, int height, int channels) {
    Image* img = create_image(width, height, channels);
    if (!img) return NULL;

    if (channels == 3) {
        PixelRGB* data = (PixelRGB*)img->data;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = y * width + x;
                data[index].r = (x * 255) / width;
                data[index].g = (y * 255) / height;
                data[index].b = ((x + y) * 255) / (width + height);
            }
        }
    } else {
        PixelGray* data = (PixelGray*)img->data;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                data[y * width + x] = ((x + y) * 255) / (width + height);
            }
        }
    }

    return img;
}

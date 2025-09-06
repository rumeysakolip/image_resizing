#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_resize.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Load an image from file using stb_image
Image* load_image(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (!data) {
        printf("Error loading image: %s\n", filename);
        return NULL;
    }

    printf("Loaded image: %s (%dx%d, %d channels)\n", filename, width, height, channels);

    // Convert 4-channel images to 3-channel (remove alpha)
    if (channels == 4) {
        printf("Converting RGBA to RGB\n");
        unsigned char* rgb_data = malloc(width * height * 3);
        for (int i = 0, j = 0; i < width * height * 4; i += 4, j += 3) {
            rgb_data[j] = data[i];
            rgb_data[j+1] = data[i+1];
            rgb_data[j+2] = data[i+2];
        }
        stbi_image_free(data);
        data = rgb_data;
        channels = 3;
    }

    // Create our image structure
    Image* img = create_image(width, height, channels);
    if (!img) {
        stbi_image_free(data);
        return NULL;
    }

    // Copy the data
    memcpy(img->data, data, width * height * (channels == 3 ? sizeof(PixelRGB) : sizeof(PixelGray)));

    // Free stb image data
    stbi_image_free(data);

    return img;
}

// Save an image to PNG file using stb_image_write
int save_image(const Image* img, const char* filename) {
    if (!img || !img->data) {
        printf("Invalid image data\n");
        return 0;
    }

    int result;
    if (img->channels == 3) {
        result = stbi_write_png(filename, img->width, img->height, 3, img->data, img->width * 3);
    } else {
        result = stbi_write_png(filename, img->width, img->height, 1, img->data, img->width);
    }

    if (result) {
        printf("Saved image: %s (%dx%d, %d channels)\n", filename, img->width, img->height, img->channels);
    } else {
        printf("Error saving image: %s\n", filename);
    }

    return result;
}

int main(int argc, char* argv[]) {
    printf("Integer-Based Image Resizing with PNG I/O\n");

    Image* original = NULL;

    // Check if an input image was provided
    if (argc > 1) {
        // Load image from file
        original = load_image(argv[1]);
    }

    // If no image was loaded, create a test pattern
    if (!original) {
        printf("No input image provided. Creating test pattern.\n");
        int width = 640;
        int height = 480;
        int channels = 3; // RGB

        original = create_test_pattern(width, height, channels);
        if (!original) {
            printf("Failed to create test image\n");
            return 1;
        }

        // Save the test pattern
        save_image(original, "test_pattern.png");
    }

    printf("Original image: %dx%d, %d channels\n", original->width, original->height, original->channels);

    // Save the original image
    save_image(original, "original.png");

    // Test different scale factors
    struct {
        int num;
        int denom;
        const char* name;
    } scales[] = {
        {1, 2, "half"},
        {1, 4, "quarter"},
        {3, 4, "three_quarters"},
        {2, 1, "double"}
    };

    for (int i = 0; i < sizeof(scales) / sizeof(scales[0]); i++) {
        char filename[256];

        // Test bilinear interpolation
        Image* resized = resize_image_fixed(original, scales[i].num, scales[i].denom);

        if (resized) {
            printf("Resized (%s): %dx%d\n", scales[i].name, resized->width, resized->height);

            // Save the resized image
            snprintf(filename, sizeof(filename), "bilinear_%s.png", scales[i].name);
            save_image(resized, filename);

            free_image(resized);
        } else {
            printf("Failed to resize image with scale %d/%d\n", scales[i].num, scales[i].denom);
        }

        // Test nearest neighbor
        Image* resized_nn = resize_image_nearest(original, scales[i].num, scales[i].denom);

        if (resized_nn) {
            printf("Resized NN (%s): %dx%d\n", scales[i].name, resized_nn->width, resized_nn->height);

            // Save the resized image
            snprintf(filename, sizeof(filename), "nearest_%s.png", scales[i].name);
            save_image(resized_nn, filename);

            free_image(resized_nn);
        } else {
            printf("Failed to resize image (NN) with scale %d/%d\n", scales[i].num, scales[i].denom);
        }
    }

    free_image(original);
    return 0;
}

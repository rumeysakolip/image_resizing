#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_resize.h"

// Define STB_IMAGE_IMPLEMENTATION and STB_IMAGE_WRITE_IMPLEMENTATION exactly once
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

// Create a gradient test image
Image* create_gradient_image(int width, int height, int channels) {
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

int main(int argc, char* argv[]) {
    printf("Image Resizing with PNG I/O\n");

    Image* input = NULL;
    Image* resized = NULL;

    // Check if an input image was provided
    if (argc > 1) {
        // Load image from file
        input = load_image(argv[1]);
    }

    // If no image was loaded, create a test gradient
    if (!input) {
        printf("Creating test gradient image\n");
        input = create_gradient_image(640, 480, 3); // RGB gradient
        if (!input) {
            printf("Failed to create test image\n");
            return 1;
        }
        save_image(input, "input.png");
    }

    printf("Original image: %dx%d, %d channels\n", input->width, input->height, input->channels);

    // Resize the image with different scale factors
    float scales[] = {0.5f, 0.25f, 0.1f};
    char filename[256];

    for (int i = 0; i < sizeof(scales) / sizeof(scales[0]); i++) {
        resized = resize_image(input, scales[i]);

        if (resized) {
            printf("Resized image: %dx%d (scale: %.2f)\n",
                   resized->width, resized->height, scales[i]);

            // Create output filename
            snprintf(filename, sizeof(filename), "output_%.2f.png", scales[i]);
            save_image(resized, filename);

            free_image(resized);
            resized = NULL;
        } else {
            printf("Failed to resize image with scale %.2f\n", scales[i]);
        }
    }

    free_image(input);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int Gx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int Gy[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

void convolve_sobel_color(unsigned char *input, unsigned char *output, int width, int height) 
{
    int k_offset = 1;
    int channels = 3; // 3 channels - red blue green

    for (int i = k_offset; i < height - k_offset; i++) {
        for (int j = k_offset; j < width - k_offset; j++) {
            
            // looping through each color channel separately
            for (int c = 0; c < channels; c++) {
                float sum_x = 0.0f;
                float sum_y = 0.0f;
                
                for (int ki = 0; ki < 3; ki++) {
                    for (int kj = 0; kj < 3; kj++) {
                        int pixel_row = i + (ki - k_offset);
                        int pixel_col = j + (kj - k_offset);
                        
                        // position in flat interleaved array is (Row * Width + Col) * 3 + Channel
                        int pixel_idx = (pixel_row * width + pixel_col) * channels + c;
                        unsigned char pixel_val = input[pixel_idx];
                        
                        sum_x += (float)pixel_val * Gx[ki][kj];
                        sum_y += (float)pixel_val * Gy[ki][kj];
                    }
                }
                
                float magnitude = sqrtf((sum_x * sum_x) + (sum_y * sum_y));
                
                // multiplying edge intensity by 2.0 to make neon bloom stand out :)
                magnitude *= 2.0f; 
                
                if (magnitude > 255.0f) magnitude = 255.0f;
                if (magnitude < 0.0f) magnitude = 0.0f;
                
                // Write directly back to corresponding output color byte
                int out_idx = (i * width + j) * channels + c;
                output[out_idx] = (unsigned char)magnitude;
            }
        }
    }
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_image_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_path = argv[1];
    int width, height, actual_channels;

    printf("Loading color image: %s...\n", input_path);
    // Request 3 channels (RGB) explicitly
    unsigned char *input_image = stbi_load(input_path, &width, &height, &actual_channels, 3);
    
    if (!input_image) 
    {
        fprintf(stderr, "Error: Could not load image file.\n");
        return EXIT_FAILURE;
    }

        
    // Allocate the output image memory buffer (Width * Height * 3 bytes)
    unsigned char *output_image = (unsigned char*)calloc(width * height * 3, sizeof(unsigned char));

    printf("Running Sobel operator...\n");
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    convolve_sobel_color(input_image, output_image, width, height);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken sequentially: %.6f seconds\n", elapsed_time);

    // Save output 
    char *output_path = "output_sobel_sequential.bmp";
    stbi_write_bmp(output_path, width, height, 3, output_image);
    printf("Saved edge visual to '%s'\n", output_path);

    stbi_image_free(input_image);
    free(output_image);
    
    return 0;
}

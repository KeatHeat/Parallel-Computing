// PARALLEL SOBEL EDGE DETECTION USING OPENMP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define CHANNELS 3  // 3 channels - red green blue
#define K_OFFSET 1  // 3x3 kernel, so skip 1 pixel on each border

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

// Computes the Sobel value for one pixel (i, j) on every color channel
void sobel_pixel(unsigned char *input, unsigned char *output, int width, int i, int j)
{
    // looping through each color channel separately
    for (int c = 0; c < CHANNELS; c++) {
        float sum_x = 0.0f;
        float sum_y = 0.0f;

        for (int ki = 0; ki < 3; ki++) {
            for (int kj = 0; kj < 3; kj++) {
                int pixel_row = i + (ki - K_OFFSET);
                int pixel_col = j + (kj - K_OFFSET);

                // position in flat interleaved array is (Row * Width + Col) * 3 + Channel
                int pixel_idx = (pixel_row * width + pixel_col) * CHANNELS + c;
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
        int out_idx = (i * width + j) * CHANNELS + c;
        output[out_idx] = (unsigned char)magnitude;
    }
}

// Sequential Sobel (baseline for benchmarking)
void convolve_sobel_seq(unsigned char *input, unsigned char *output, int width, int height)
{
    for (int i = K_OFFSET; i < height - K_OFFSET; i++) {
        for (int j = K_OFFSET; j < width - K_OFFSET; j++) {
            sobel_pixel(input, output, width, i, j);
        }
    }
}

// Parallel Sobel: collapse(2) merges the row and column loops into one
// big loop of (height - 2) * (width - 2) pixels, and OpenMP splits those
// pixels evenly between the threads. Every pixel is independent, so
// there are no shared writes and no locking is needed.
void convolve_sobel_omp(unsigned char *input, unsigned char *output, int width, int height)
{
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = K_OFFSET; i < height - K_OFFSET; i++) {
        for (int j = K_OFFSET; j < width - K_OFFSET; j++) {
            sobel_pixel(input, output, width, i, j);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_image_path> <num_of_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_path = argv[1];
    int num_threads = atoi(argv[2]);

    if (num_threads < 1) {
        fprintf(stderr, "Thread count must be at least 1.\n");
        return EXIT_FAILURE;
    }

    // Set the number of threads used by every parallel region
    omp_set_num_threads(num_threads);

    int width, height, actual_channels;

    printf("Loading color image: %s...\n", input_path);
    // Request 3 channels (RGB) explicitly
    unsigned char *input_image = stbi_load(input_path, &width, &height, &actual_channels, CHANNELS);

    if (!input_image) {
        fprintf(stderr, "Error: Could not load image file.\n");
        return EXIT_FAILURE;
    }

    // One output buffer for each version so the results can be compared
    unsigned char *seq_output = (unsigned char *)calloc(width * height * CHANNELS, sizeof(unsigned char));
    unsigned char *par_output = (unsigned char *)calloc(width * height * CHANNELS, sizeof(unsigned char));

    if (!seq_output || !par_output) {
        perror("calloc");
        stbi_image_free(input_image);
        return EXIT_FAILURE;
    }

    // omp_get_wtime() is OpenMP's wall clock timer
    double start, end;

    // ----- sequential baseline -----
    printf("Running Sobel operator sequentially...\n");
    start = omp_get_wtime();
    convolve_sobel_seq(input_image, seq_output, width, height);
    end = omp_get_wtime();
    printf("Time taken sequentially: %.6f seconds\n", end - start);

    // ----- parallel OpenMP version -----
    printf("Running Sobel operator with %d OpenMP thread(s)...\n", num_threads);
    start = omp_get_wtime();
    convolve_sobel_omp(input_image, par_output, width, height);
    end = omp_get_wtime();
    printf("Time taken with OpenMP (%d threads): %.6f seconds\n", num_threads, end - start);

    // Check that the parallel output matches the sequential output
    int match = memcmp(seq_output, par_output, width * height * CHANNELS) == 0;
    printf("Parallel output matches sequential: %s\n", match ? "YES" : "NO");

    // Save output
    char *output_path = "output_sobel_openmp.bmp";
    stbi_write_bmp(output_path, width, height, CHANNELS, par_output);
    printf("Saved edge visual to '%s'\n", output_path);

    stbi_image_free(input_image);
    free(seq_output);
    free(par_output);

    return 0;
}

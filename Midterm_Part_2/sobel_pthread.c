// PARALLEL SOBEL EDGE DETECTION USING PTHREADS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

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

// Data passed to each thread: the images and the rows it is responsible for
typedef struct {
    unsigned char *input;
    unsigned char *output;
    int width;
    int height;
    int start_row;
    int end_row;    // exclusive
} ThreadData;

// Runs the Sobel operator on rows [start_row, end_row) of the image.
// Used by both the sequential baseline and each pthread.
void convolve_sobel_rows(unsigned char *input, unsigned char *output,
                         int width, int start_row, int end_row)
{
    for (int i = start_row; i < end_row; i++) {
        for (int j = K_OFFSET; j < width - K_OFFSET; j++) {

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
    }
}

// Thread function: each thread runs Sobel on its own block of rows.
// Threads only read the shared input and write to their own rows of
// the output, so no locking is needed.
void *parallel_sobel(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    convolve_sobel_rows(data->input, data->output, data->width,
                        data->start_row, data->end_row);

    return NULL;
}

double elapsed_sec(struct timespec s, struct timespec e)
{
    return (e.tv_sec - s.tv_sec) + (e.tv_nsec - s.tv_nsec) / 1e9;
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

    struct timespec start, end;

    // ----- sequential baseline -----
    printf("Running Sobel operator sequentially...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    convolve_sobel_rows(input_image, seq_output, width, K_OFFSET, height - K_OFFSET);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Time taken sequentially: %.6f seconds\n", elapsed_sec(start, end));

    // ----- parallel pthread version -----
    // Only the interior rows (1 to height - 2) are processed
    int total_rows = height - 2 * K_OFFSET;

    // Never make more threads than there are rows to work on
    if (num_threads > total_rows) {
        num_threads = total_rows;
    }

    pthread_t threads[num_threads];
    ThreadData tdata[num_threads];

    // Split the rows evenly. The first (total_rows % num_threads) threads
    // get one extra row so the leftover rows are spread out instead of
    // all landing on the last thread.
    int rows_per_thread = total_rows / num_threads;
    int remainder = total_rows % num_threads;

    printf("Running Sobel operator with %d pthread(s)...\n", num_threads);
    clock_gettime(CLOCK_MONOTONIC, &start);

    int current_row = K_OFFSET;
    for (int i = 0; i < num_threads; i++) {
        int rows = rows_per_thread + (i < remainder ? 1 : 0);

        tdata[i].input = input_image;
        tdata[i].output = par_output;
        tdata[i].width = width;
        tdata[i].height = height;
        tdata[i].start_row = current_row;
        tdata[i].end_row = current_row + rows;
        current_row += rows;

        if (pthread_create(&threads[i], NULL, parallel_sobel, &tdata[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for every thread to finish its rows
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Time taken with pthreads (%d threads): %.6f seconds\n",
           num_threads, elapsed_sec(start, end));

    // Check that the parallel output matches the sequential output
    int match = memcmp(seq_output, par_output, width * height * CHANNELS) == 0;
    printf("Parallel output matches sequential: %s\n", match ? "YES" : "NO");

    // Save output
    char *output_path = "output_sobel_pthread.bmp";
    stbi_write_bmp(output_path, width, height, CHANNELS, par_output);
    printf("Saved edge visual to '%s'\n", output_path);

    stbi_image_free(input_image);
    free(seq_output);
    free(par_output);

    return 0;
}

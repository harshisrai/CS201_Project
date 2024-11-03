#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to read the entire file into memory
char *read_file(const char *filename, size_t *file_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    char *data = (char *)malloc(*file_size + 1);
    if (!data) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(data, 1, *file_size, file);
    data[*file_size] = '\0'; // Null-terminate for safety with string functions
    fclose(file);

    return data;
}

// Function to write data to a file
void write_file(const char *filename, const char *data) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }
    fwrite(data, 1, strlen(data), file);
    fclose(file);
}

// Function to perform Run-Length Encoding (compression)
void rle_compress(const char *input, char *output) {
    int count, i, j = 0;
    int length = strlen(input);

    for (i = 0; i < length; i++) {
        // If the character is a digit, add `_` marker and copy all consecutive digits as-is
        if (isdigit(input[i])) {
            output[j++] = '_';
            while (i < length && isdigit(input[i])) {
                output[j++] = input[i++];
            }
            i--; // Adjust the position since the for loop will increment `i` again
            continue;
        }

        // Count occurrences of the current non-digit character
        count = 1;
        while (i + 1 < length && input[i] == input[i + 1] && !isdigit(input[i])) {
            count++;
            i++;
        }

        // Store the character
        output[j++] = input[i];

        // Store the count if greater than 1
        if (count > 1) {
            char countStr[10];
            sprintf(countStr, "%d", count);
            for (int k = 0; countStr[k] != '\0'; k++) {
                output[j++] = countStr[k];
            }
        }
    }
    output[j] = '\0'; // Null-terminate the output string
}

// Function to perform Run-Length Decoding (decompression)
void rle_decompress(const char *input, char *output) {
    int i, j = 0;
    int length = strlen(input);

    for (i = 0; i < length; i++) {
        // Check if the current character is an underscore `_` indicating raw numbers follow
        if (input[i] == '_') {
            i++; // Move to the first digit after `_`
            while (i < length && isdigit(input[i])) {
                output[j++] = input[i++];
            }
            i--; // Adjust the position since the for loop will increment `i` again
            continue;
        }

        // Copy the character to output
        char current_char = input[i];
        output[j++] = current_char;

        // Check if the next characters are digits (representing the count)
        int count = 0;
        while (i + 1 < length && isdigit(input[i + 1])) {
            count = count * 10 + (input[++i] - '0');
        }

        // Repeat the character (count - 1) times
        for (int k = 1; k < count; k++) {
            output[j++] = current_char;
        }
    }
    output[j] = '\0'; // Null-terminate the output string
}

int main() {
    size_t input_length;
    char *input = read_file("sample.txt", &input_length);
    if (!input) {
        return 1;
    }

    // Allocate memory for compressed output (twice the input length is a safe estimate)
    char *compressed = (char *)malloc(2 * input_length + 1);
    if (!compressed) {
        perror("Memory allocation failed for compressed data");
        free(input);
        return 1;
    }
    rle_compress(input, compressed);

    // Write compressed data to compressed.txt
    write_file("compressed.txt", compressed);

    // Free the input data, as it's no longer needed
    free(input);

    // Allocate memory for decompressed output
    char *decompressed = (char *)malloc(input_length + 1); // Should be enough to hold the original size
    if (!decompressed) {
        perror("Memory allocation failed for decompressed data");
        free(compressed);
        return 1;
    }

    // Decompress the compressed data
    rle_decompress(compressed, decompressed);

    // Write decompressed data to decompressed.txt
    write_file("decompressed.txt", decompressed);

    // Print sizes of the files
    printf("Size of sample.txt: %zu bytes\n", input_length);
    printf("Size of compressed.txt: %zu bytes\n", strlen(compressed));
    printf("Size of decompressed.txt: %zu bytes\n", strlen(decompressed));

    // Free the compressed and decompressed data
    free(compressed);
    free(decompressed);

    printf("Compression and decompression completed.\n");
    return 0;
}

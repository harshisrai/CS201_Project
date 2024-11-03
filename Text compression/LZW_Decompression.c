#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT_SIZE 4096   // Maximum dictionary size for LZW (12-bit codes)
#define INIT_DICT_SIZE 256   // Initial dictionary size (ASCII)

// Function to perform LZW decompression
char* LZWDecompress(int* codes, int num_codes) {
    // Initialize the dictionary with single-character strings (ASCII)
    char* dictionary[MAX_DICT_SIZE];
    for (int i = 0; i < INIT_DICT_SIZE; i++) {
        dictionary[i] = (char*)malloc(2 * sizeof(char));
        dictionary[i][0] = (char)i;
        dictionary[i][1] = '\0';
    }
    
    int dict_size = INIT_DICT_SIZE;
    
    // Initial output buffer size (start larger for large files)
    int output_buffer_size = 65536; // Start with 64KB
    char* output = (char*)malloc(output_buffer_size * sizeof(char)); 
    int output_index = 0;
    
    // Get the first code
    int OLD = codes[0];
    strcpy(output, dictionary[OLD]);
    output_index += strlen(dictionary[OLD]);

    char C = dictionary[OLD][0]; // First character of OLD's translation
    
    // Loop through the rest of the codes
    for (int i = 1; i < num_codes; i++) {
        int NEW = codes[i];
        char S[1024]; // Temporary buffer for the translation

        // If NEW is not in the dictionary
        if (NEW >= dict_size) {
            // S = translation of OLD + C
            strcpy(S, dictionary[OLD]);
            S[strlen(dictionary[OLD])] = C;
            S[strlen(dictionary[OLD]) + 1] = '\0';
        } else {
            // S = translation of NEW
            strcpy(S, dictionary[NEW]);
        }

        // Check if output buffer needs resizing
        if (output_index + strlen(S) >= output_buffer_size) {
            output_buffer_size *= 2;
            output = (char*)realloc(output, output_buffer_size * sizeof(char));
        }

        // Output S
        strcat(output, S);
        output_index += strlen(S);

        // C = first character of S
        C = S[0];

        // Add OLD + C to the dictionary
        char new_entry[1024];
        strcpy(new_entry, dictionary[OLD]);
        int len = strlen(dictionary[OLD]);
        new_entry[len] = C;
        new_entry[len + 1] = '\0';

        if (dict_size < MAX_DICT_SIZE) {
            dictionary[dict_size] = strdup(new_entry);
            dict_size++;
        }

        // OLD = NEW
        OLD = NEW;
    }

    return output;
}


// Function to read an integer array from a .bin file
int* readArrayFromBinFile(const char *filename, int *size) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file for reading.\n");
        exit(1);
    }

    // Find the size of the file
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Calculate the number of integers in the file
    *size = fileSize / sizeof(int);

    // Allocate memory for the array
    int *arr = (int*)malloc(fileSize);
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        exit(1);
    }

    // Read the file content into the array
    fread(arr, sizeof(int), *size, file);
    fclose(file);

    return arr;
}

// Example usage
int main() {
    // Read the compressed codes from a binary file
    int num_codes;
    int* codes = readArrayFromBinFile("compressed.bin", &num_codes);

    // Perform LZW decompression
    char* decompressed_str = LZWDecompress(codes, num_codes);

    //write to a .txt
    FILE* file=fopen("decompressed.txt", "w");
    fputs(decompressed_str,file);
    fclose(file);

    // Free the memory allocated for the decompressed string
    free(decompressed_str);
    //store the sizes of each file in bytes in a double variable
    FILE *file1 = fopen("compressed.bin", "r");
    FILE *file2 = fopen("decompressed.txt", "r");
    fseek(file1, 0, SEEK_END);
    double size1= (double)ftell(file1);
    fseek(file2, 0, SEEK_END);
    double size2= (double)ftell(file2);
    printf("Size of compressed file: %f KB\n", size1/1000);
    printf("Size of decompressed file: %f KB\n", size2/1000);

    return 0;
}

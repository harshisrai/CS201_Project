#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT_SIZE 4096   // LZW dictionary size (12-bit)
#define INIT_DICT_SIZE 256   // Initial dictionary size (ASCII)
#define HASH_TABLE_SIZE 8192 // Hash table size, typically larger than dictionary size

// Structure to represent a dictionary entry with linked list (for collision handling)
typedef struct DictionaryEntry {
    char *str;
    int code;
    struct DictionaryEntry *next;
} DictionaryEntry;

// Hash function for strings (djb2 algorithm)
unsigned int hashFunction(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash % HASH_TABLE_SIZE;
}

// Function to initialize the dictionary with single-character strings (ASCII)
void initDictionary(DictionaryEntry **hashTable) {
    for (int i = 0; i < INIT_DICT_SIZE; i++) {
        DictionaryEntry *entry = (DictionaryEntry *)malloc(sizeof(DictionaryEntry));
        entry->str = (char *)malloc(2 * sizeof(char));
        entry->str[0] = i;
        entry->str[1] = '\0';
        entry->code = i;
        entry->next = NULL;

        unsigned int hashIndex = hashFunction(entry->str);
        entry->next = hashTable[hashIndex];
        hashTable[hashIndex] = entry;
    }
}

// Function to find a string in the dictionary (hash table lookup)
int findInDictionary(DictionaryEntry **hashTable, const char *str) {
    unsigned int hashIndex = hashFunction(str);
    DictionaryEntry *entry = hashTable[hashIndex];

    while (entry != NULL) {
        if (strcmp(entry->str, str) == 0) {
            return entry->code;
        }
        entry = entry->next;
    }

    return -1; // Not found
}

// Function to add a new entry to the dictionary
void addToDictionary(DictionaryEntry **hashTable, const char *str, int code) {
    DictionaryEntry *entry = (DictionaryEntry *)malloc(sizeof(DictionaryEntry));
    entry->str = strdup(str);
    entry->code = code;
    entry->next = NULL;

    unsigned int hashIndex = hashFunction(str);
    entry->next = hashTable[hashIndex];
    hashTable[hashIndex] = entry;
}

// Function to perform LZW compression on the input string
int *LZWCompress(const char *input, int *output_size) {
    // Initialize the hash table and dictionary
    DictionaryEntry *hashTable[HASH_TABLE_SIZE] = {NULL};
    initDictionary(hashTable);
    
    int dict_size = INIT_DICT_SIZE;
    char P[1024] = ""; // Previous string

    // Initial output buffer size (larger for large files)
    int output_buffer_size = 65536; // Start with 64KB
    int *output = (int *)malloc(output_buffer_size * sizeof(int)); // Allocate memory for output
    int output_index = 0;

    // Start with the first input character
    P[0] = input[0];
    P[1] = '\0';
    int input_length = strlen(input);

    // Loop through the input characters
    for (int i = 1; i <= input_length; i++) {
        char C[2] = {input[i], '\0'}; // Current character

        // Create P + C (concatenation of P and C)
        char PC[1024];
        strcpy(PC, P);
        strcat(PC, C);

        // Check if P + C exists in the dictionary
        int dict_code = findInDictionary(hashTable, PC);
        if (dict_code != -1) {
            // P = P + C
            strcpy(P, PC);
        } else {
            // Output the code for P
            if (output_index >= output_buffer_size) {
                // Resize the output buffer dynamically
                output_buffer_size *= 2;
                output = (int *)realloc(output, output_buffer_size * sizeof(int));
            }
            output[output_index++] = findInDictionary(hashTable, P);

            // Add P + C to the dictionary
            if (dict_size < MAX_DICT_SIZE) {
                addToDictionary(hashTable, PC, dict_size++);
            }

            // P = C
            strcpy(P, C);
        }
    }

    // Output the code for the last P
    output[output_index++] = findInDictionary(hashTable, P);

    *output_size = output_index;
    return output;
}


// Function to read a file and return its contents as a single string
char* readFileToString(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // Seek to the end of the file to find the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file); // Go back to the beginning of the file

    // Allocate memory to store the file contents + 1 (for null terminator)
    char *content = (char*)malloc((file_size + 1) * sizeof(char));
    if (content == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // Read the entire file into the buffer
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0'; // Null-terminate the string

    fclose(file); // Close the file
    return content; // Return the file contents as a string
}

void saveArrayToBinFile(const char *filename, int *arr, size_t size) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        exit(1);
    }

    fwrite(arr, sizeof(int), size, file);
    fclose(file);
}

// Main function to test LZW compression
int main() {
    // Read the input file as a string
    printf("Enter the filename: ");
    char filename[256];  // Adjust the size as needed
    scanf("%255s", filename);  // Limit input to avoid overflow
    
    const char *input = readFileToString(filename);
    int output_size;
    
    // Perform LZW compression
    int *compressed_codes = LZWCompress(input, &output_size);

    // // Print the compressed LZW codes
    // printf("LZW Compressed Codes: ");
    // for (int i = 0; i < output_size; i++) {
    //     printf("%d ", compressed_codes[i]);
    // }
    // printf("\n");
    saveArrayToBinFile("compressed.bin", compressed_codes, output_size);

    // Free allocated memory
    free(compressed_codes);
    FILE *file1 = fopen("compressed.bin", "r");
    FILE *file2 = fopen(filename, "r");
    //store the sizes of each file in bytes in a double variable
    double size1, size2;
    fseek(file1, 0, SEEK_END);
    size1 = (double)ftell(file1);
    fseek(file2, 0, SEEK_END);
    size2 = (double)ftell(file2);
    //print the sizes of each file
    printf("Size of compressed file: %f KB\n", size1/1000);
    printf("Size of original file: %f KB\n", size2/1000);
    
    return 0;
}

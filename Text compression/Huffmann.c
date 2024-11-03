#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 256  // Maximum possible characters (ASCII range)

// Huffman Tree Node
typedef struct HuffmanNode {
    char character;
    int frequency;
    struct HuffmanNode *left, *right;
} HuffmanNode;

// Min Heap Structure
typedef struct MinHeap {
    int size;
    int capacity;
    HuffmanNode** array;
} MinHeap;

// Function to create a new Huffman node
HuffmanNode* createNode(char character, int frequency) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->character = character;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

// Function to create a min-heap with given capacity
MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (HuffmanNode**)malloc(minHeap->capacity * sizeof(HuffmanNode*));
    return minHeap;
}

// Swap two nodes in the min-heap
void swapNodes(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

// Function to maintain the min-heap property
void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->frequency < minHeap->array[smallest]->frequency) {
        smallest = left;
    }

    if (right < minHeap->size && minHeap->array[right]->frequency < minHeap->array[smallest]->frequency) {
        smallest = right;
    }

    if (smallest != idx) {
        swapNodes(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// Function to check if the size of the heap is 1
int isSizeOne(MinHeap* minHeap) {
    return (minHeap->size == 1);
}

// Function to insert a node into the min-heap
void insertMinHeap(MinHeap* minHeap, HuffmanNode* node) {
    minHeap->size++;
    int i = minHeap->size - 1;

    while (i && node->frequency < minHeap->array[(i - 1) / 2]->frequency) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = node;
}

// Function to extract the node with the minimum frequency from the heap
HuffmanNode* extractMin(MinHeap* minHeap) {
    HuffmanNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;
    minHeapify(minHeap, 0);
    return temp;
}

// Function to build the min-heap
void buildMinHeap(MinHeap* minHeap) {
    int n = minHeap->size - 1;
    for (int i = (n - 1) / 2; i >= 0; --i) {
        minHeapify(minHeap, i);
    }
}

// Function to create and build a min-heap from characters and their frequencies
MinHeap* createAndBuildMinHeap(char characters[], int freq[], int size) {
    MinHeap* minHeap = createMinHeap(size);

    for (int i = 0; i < size; i++) {
        minHeap->array[i] = createNode(characters[i], freq[i]);
    }

    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

// Function to build the Huffman Tree
HuffmanNode* buildHuffmanTree(char characters[], int freq[], int size) {
    HuffmanNode *left, *right, *top;

    MinHeap* minHeap = createAndBuildMinHeap(characters, freq, size);

    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        top = createNode('$', left->frequency + right->frequency);

        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }

    return extractMin(minHeap);
}

// Array to store the Huffman codes for each character
char huffmanCodes[MAX_CHARS][MAX_CHARS];

// Function to store Huffman codes in an array
void storeCodes(HuffmanNode* root, int arr[], int top) {
    if (root->left) {
        arr[top] = 0;
        storeCodes(root->left, arr, top + 1);
    }

    if (root->right) {
        arr[top] = 1;
        storeCodes(root->right, arr, top + 1);
    }

    if (!(root->left) && !(root->right)) {
        for (int i = 0; i < top; i++) {
            huffmanCodes[(unsigned char)root->character][i] = arr[i] + '0'; // Store as a string
        }
        huffmanCodes[(unsigned char)root->character][top] = '\0';  // Null terminate the string
    }
}

// Function to read the file and calculate character frequencies
void calculateFrequencies(const char* filename, int freq[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    char c;
    while ((c = fgetc(file)) != EOF) {
        freq[(unsigned char)c]++;
    }

    fclose(file);
}

// Function to write the encoded data into a new file
void writeEncodedFile(const char* inputFilename, const char* outputFilename) {
    FILE *inputFile = fopen(inputFilename, "r");
    FILE *outputFile = fopen(outputFilename, "wb");  // Write in binary mode

    if (!inputFile || !outputFile) {
        printf("Error opening files.\n");
        return;
    }

    char c;
    unsigned char buffer = 0;
    int bitCount = 0;

    // Traverse the input file
    while ((c = fgetc(inputFile)) != EOF) {
        // Get the Huffman code for the current character
        char* code = huffmanCodes[(unsigned char)c];

        // Write each bit of the code
        for (int i = 0; i < strlen(code); i++) {
            if (code[i] == '1') {
                buffer |= (1 << (7 - bitCount));  // Set bit if it's '1'
            }
            bitCount++;

            // If we filled 8 bits, write the byte to the file
            if (bitCount == 8) {
                fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    // Write any remaining bits
    if (bitCount > 0) {
        fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
}
// Function to insert the code into the Huffman Tree
void insertCode(HuffmanNode* root, const char* code, char character) {
    HuffmanNode* current = root;
    
    for (int i = 0; code[i] != '\0'; i++) {
        if (code[i] == '0') {
            if (!current->left) {
                current->left = createNode('\0', 0);  // Create left child with dummy character and frequency 0
            }
            current = current->left;
        } else if (code[i] == '1') {
            if (!current->right) {
                current->right = createNode('\0', 0);  // Create right child with dummy character and frequency 0
            }
            current = current->right;
        }
    }

    // Assign the character to the leaf node
    current->character = character;
    current->frequency = 1; // Set a default frequency for the leaf node
}


// Function to decode the encoded binary file
void decodeFile(const char* encodedFilename, const char* outputFilename, HuffmanNode* root) {
    FILE *encodedFile = fopen(encodedFilename, "rb");
    FILE *outputFile = fopen(outputFilename, "w");

    if (!encodedFile || !outputFile) {
        printf("Error opening files.\n");
        return;
    }

    HuffmanNode* current = root;
    unsigned char byte;
    while (fread(&byte, sizeof(unsigned char), 1, encodedFile) == 1) {
        for (int i = 0; i < 8; i++) {
            int bit = (byte >> (7 - i)) & 1;
            if (bit == 0) {
                current = current->left;
            } else {
                current = current->right;
            }

            // If we reach a leaf node, write the character to output
            if (!current->left && !current->right) {
                fputc(current->character, outputFile);
                current = root;
            }
        }
    }

    fclose(encodedFile);
    fclose(outputFile);
}
int main() {
    int freq[MAX_CHARS] = {0};  // Array to store the frequency of each character
    const char* inputFilename = "sample.txt";       // Original text file
    const char* encodedFilename = "compressed.bin"; // Encoded binary file
    const char* decodedFilename = "decoded.txt";  // File to store the decoded text

    // Read the text file and calculate the frequencies
    calculateFrequencies(inputFilename, freq);

    // Count how many unique characters have non-zero frequency
    int size = 0;
    for (int i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            size++;
        }
    }

    // Allocate memory for characters and their frequencies
    char* characters = (char*)malloc(size * sizeof(char));
    int* frequencies = (int*)malloc(size * sizeof(int));

    // Populate the character and frequency arrays
    int index = 0;
    for (int i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            characters[index] = (char)i;
            frequencies[index] = freq[i];
            index++;
        }
    }

    // Build the Huffman Tree
    HuffmanNode* root = buildHuffmanTree(characters, frequencies, size);

    // Generate the Huffman codes
    int arr[100], top = 0;
    storeCodes(root, arr, top);

    // Write the encoded file
    writeEncodedFile(inputFilename, encodedFilename);

    // Decode the file
    decodeFile(encodedFilename, decodedFilename, root);

    // Clean up
    free(characters);
    free(frequencies);

    printf("Compression and decoding completed. Check the output file: %s\n", decodedFilename);
    
    // Free the Huffman Tree (additional cleanup)
    free(root);

    return 0;
}

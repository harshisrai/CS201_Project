#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_COLORS 16777216 // 256^3 for 24-bit RGB colors

#pragma pack(push, 1)

// BMP file header (14 bytes)
typedef struct {
    unsigned char bfType[2];        // File type
    unsigned int bfSize;            // Size of the file
    unsigned short bfReserved1;     // Reserved; must be zero
    unsigned short bfReserved2;     // Reserved; must be zero
    unsigned int bfOffBits;         // Offset to start of pixel data
} BMPFileHeader;

// DIB header (typically 40 bytes for BITMAPINFOHEADER)
typedef struct {
    unsigned int biSize;            // Size of the header
    int biWidth;                    // Width of the image
    int biHeight;                   // Height of the image
    unsigned short biPlanes;        // Number of color planes
    unsigned short biBitCount;      // Bits per pixel
    unsigned int biCompression;     // Compression type
    unsigned int biSizeImage;       // Size of the image data
    int biXPelsPerMeter;             // Horizontal resolution
    int biYPelsPerMeter;             // Vertical resolution
    unsigned int biClrUsed;         // Number of colors in the color palette
    unsigned int biClrImportant;    // Important colors
} BMPDIBHeader;

#pragma pack(pop)

void readBMP(const char *filename, unsigned char **pixelData, int *width, int *height, unsigned int *colorFrequency) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening BMP file");
        return;
    }

    // Read BMP file header
    BMPFileHeader fileHeader;
    fread(&fileHeader, sizeof(BMPFileHeader), 1, file);

    // Verify if it's a BMP file
    if (fileHeader.bfType[0] != 'B' || fileHeader.bfType[1] != 'M') {
        printf("Not a valid BMP file.\n");
        fclose(file);
        return;
    }

    // Read DIB header
    BMPDIBHeader dibHeader;
    fread(&dibHeader, sizeof(BMPDIBHeader), 1, file);

    // Check for valid bit count (only 24-bit is supported)
    if (dibHeader.biBitCount != 24) {
        printf("Only 24-bit BMP files are supported. Found: %u bits.\n", dibHeader.biBitCount);
        fclose(file);
        return;
    }

    // Check for uncompressed BMP (compression type 0)
    if (dibHeader.biCompression != 0) {
        printf("Only uncompressed BMP files are supported. Found compression type: %u.\n", dibHeader.biCompression);
        fclose(file);
        return;
    }

    // Set width and height
    *width = dibHeader.biWidth;
    *height = dibHeader.biHeight;

    // Calculate the padded row size (each row is padded to a multiple of 4 bytes)
    int row_padded = (*width * 3 + 3) & (~3);
    
    // Allocate memory for pixel data
    *pixelData = (unsigned char *)malloc(row_padded * (*height));
    if (*pixelData == NULL) {
        perror("Error allocating memory for pixel data");
        fclose(file);
        return;
    }

    // Move the file pointer to the start of pixel data
    fseek(file, fileHeader.bfOffBits, SEEK_SET);

    // Read pixel data
    size_t bytesRead = fread(*pixelData, sizeof(unsigned char), row_padded * (*height), file);
    if (bytesRead != row_padded * (*height)) {
        printf("Error reading pixel data from BMP file. Bytes read: %zu\n", bytesRead);
        free(*pixelData);
        fclose(file);
        return;
    }

    // Process pixel data to create color frequency
    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            // Calculate the index for pixel data
            int pixelIndex = (i * row_padded) + (j * 3);
            unsigned char blue = (*pixelData)[pixelIndex];
            unsigned char green = (*pixelData)[pixelIndex + 1];
            unsigned char red = (*pixelData)[pixelIndex + 2];

            // Combine RGB into a single color value
            unsigned int color = (red << 16) | (green << 8) | blue;
            colorFrequency[color]++;
        }
    }

    printf("Successfully read the image file!\n");
    fclose(file);
}

typedef struct ColorFrequencyPair {
    unsigned int color;     // Color in RGB format
    unsigned int frequency; // Frequency of the color
} ColorFrequencyPair;

ColorFrequencyPair* createColorFrequencyPairs(unsigned int *colorFrequency, int *size) {
    int count = 0;

    // Count the number of unique colors
    for (int i = 0; i < MAX_COLORS; i++) {
        if (colorFrequency[i] > 0) {
            count++;
        }
    }

    // Allocate memory for the color frequency pairs
    ColorFrequencyPair *pairs = (ColorFrequencyPair *)malloc(count * sizeof(ColorFrequencyPair));
    if (pairs == NULL) {
        perror("Error allocating memory for color frequency pairs");
        return NULL; // Memory allocation failed
    }

    // Populate the pairs array with colors and their frequencies
    int index = 0;
    for (int i = 0; i < MAX_COLORS; i++) {
        if (colorFrequency[i] > 0) {
            pairs[index].color = i;              // Set color
            pairs[index].frequency = colorFrequency[i]; // Set frequency
            index++;
        }
    }

    *size = count; // Set the size of the color frequency pairs array
    printf("Color frequency pairs created successfully! Count: %d\n", count);
    return pairs; // Return the array of color frequency pairs
}

typedef struct HuffmanNode {
    unsigned int color;          // The color value
    unsigned int frequency;      // The frequency of the color
    struct HuffmanNode *left;    // Left child
    struct HuffmanNode *right;   // Right child
} HuffmanNode;

typedef struct {
    HuffmanNode **array; // Array of Huffman nodes (for the priority queue)
    int size;            // Current size of the queue
} MinHeap;

// Function to create a new Huffman node
HuffmanNode* createHuffmanNode(unsigned int color, unsigned int frequency) {
    HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    node->color = color;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Function to create a MinHeap for Huffman nodes
MinHeap* createMinHeap(int capacity) {
    MinHeap *minHeap = (MinHeap *)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->array = (HuffmanNode **)malloc(capacity * sizeof(HuffmanNode *));
    return minHeap;
}

// Function to swap two nodes in the MinHeap
void swapHuffmanNode(HuffmanNode **a, HuffmanNode **b) {
    HuffmanNode *temp = *a;
    *a = *b;
    *b = temp;
}

// Function to heapify the MinHeap
void minHeapify(MinHeap *minHeap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < minHeap->size && minHeap->array[left]->frequency < minHeap->array[smallest]->frequency)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->frequency < minHeap->array[smallest]->frequency)
        smallest = right;

    if (smallest != index) {
        swapHuffmanNode(&minHeap->array[smallest], &minHeap->array[index]);
        minHeapify(minHeap, smallest);
    }
}

// Function to check if the size of the MinHeap is one
int isSizeOne(MinHeap *minHeap) {
    return (minHeap->size == 1);
}

// Function to extract the minimum node from the MinHeap
HuffmanNode* extractMin(MinHeap *minHeap) {
    HuffmanNode *temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;
    minHeapify(minHeap, 0);
    return temp;
}

// Function to insert a new node into the MinHeap
void insertMinHeap(MinHeap *minHeap, HuffmanNode *node) {
    minHeap->array[minHeap->size] = node;
    int index = minHeap->size;
    minHeap->size++;

    while (index && minHeap->array[index]->frequency < minHeap->array[(index - 1) / 2]->frequency) {
        swapHuffmanNode(&minHeap->array[index], &minHeap->array[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

// Function to build the Huffman tree from color frequency pairs
void buildHuffmanTree(ColorFrequencyPair *pairs, int size, HuffmanNode **root) {
    MinHeap *minHeap = createMinHeap(size);
    
    // Create a MinHeap with all the Huffman nodes
    for (int i = 0; i < size; i++) {
        insertMinHeap(minHeap, createHuffmanNode(pairs[i].color, pairs[i].frequency));
    }

    // Create the Huffman tree
    while (!isSizeOne(minHeap)) {
        HuffmanNode *left = extractMin(minHeap);
        HuffmanNode *right = extractMin(minHeap);
        
        // Create a new internal node with these two nodes as children
        HuffmanNode *newNode = createHuffmanNode(0, left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        
        // Insert the new node back into the MinHeap
        insertMinHeap(minHeap, newNode);
    }

    // The remaining node is the root of the Huffman tree
    *root = extractMin(minHeap);
}

// Function to generate Huffman codes from the tree
void generateHuffmanCodes(HuffmanNode *root, char **codes, char *codeBuffer, int depth) {
    if (root == NULL) return;

    // If it's a leaf node, store the code and print it
    if (!root->left && !root->right) {
        codeBuffer[depth] = '\0'; // Null terminate the string
        codes[root->color] = strdup(codeBuffer); // Store the code for the color
        
        // Print the color and its Huffman code
        unsigned char red = (root->color >> 16) & 0xFF;
        unsigned char green = (root->color >> 8) & 0xFF;
        unsigned char blue = root->color & 0xFF;
        //printf("Color RGB(%u, %u, %u): Huffman Code: %s\n", red, green, blue, codeBuffer);
        return;
    }

    // Traverse left and right
    codeBuffer[depth] = '0';
    generateHuffmanCodes(root->left, codes, codeBuffer, depth + 1);
    codeBuffer[depth] = '1';
    generateHuffmanCodes(root->right, codes, codeBuffer, depth + 1);
}


void encodePixelData(unsigned char *pixelData, int width, int height, int row_padded, char **codes, unsigned char **encodedData, size_t *encodedSize) {
    // Temporary buffer for encoded bits
    size_t bufferCapacity = row_padded * height * 8; // Approximate capacity in bits
    *encodedData = (unsigned char *)malloc((bufferCapacity + 7) / 8); // Allocate space for the encoded data in bytes
    if (*encodedData == NULL) {
        perror("Error allocating memory for encoded data");
        return;
    }

    size_t bitPos = 0; // Current bit position in the encodedData buffer

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Calculate the index for pixel data
            int pixelIndex = (i * row_padded) + (j * 3);
            unsigned char blue = pixelData[pixelIndex];
            unsigned char green = pixelData[pixelIndex + 1];
            unsigned char red = pixelData[pixelIndex + 2];

            // Combine RGB into a single color value
            unsigned int color = (red << 16) | (green << 8) | blue;

            // Get the Huffman code for this color
            char *code = codes[color];
            if (code == NULL) {
                fprintf(stderr, "No Huffman code found for color %u\n", color);
                free(*encodedData);
                *encodedData = NULL;
                *encodedSize = 0;
                return;
            }

            // Write the code bit by bit to the encodedData buffer
            for (size_t k = 0; k < strlen(code); k++) {
                if (code[k] == '1') {
                    (*encodedData)[bitPos / 8] |= (1 << (7 - (bitPos % 8)));
                }
                bitPos++;
            }
        }
    }

    *encodedSize = (bitPos + 7) / 8; // Calculate the final size in bytes
}

void writeEncodedDataToFile(const char *outputFileName, unsigned char *encodedData, size_t encodedSize) {
    FILE *outputFile = fopen(outputFileName, "wb");
    if (outputFile == NULL) {
        perror("Error creating output file");
        return;
    }

    // Write the encoded data to the file
    fwrite(encodedData, sizeof(unsigned char), encodedSize, outputFile);

    printf("Encoded data successfully written to %s\n", outputFileName);

    fclose(outputFile);
}

void decodeBinaryFile(const char *encodedFileName, HuffmanNode *huffmanRoot, int width, int height, int row_padded, unsigned char *bmpHeader, int headerSize) {
    FILE *encodedFile = fopen(encodedFileName, "rb");
    if (encodedFile == NULL) {
        perror("Error opening encoded file");
        return;
    }

    unsigned char *decodedPixelData = (unsigned char *)malloc(row_padded * height);
    if (decodedPixelData == NULL) {
        perror("Error allocating memory for decoded pixel data");
        fclose(encodedFile);
        return;
    }

    unsigned char buffer;
    int bitPos = 0;
    HuffmanNode *currentNode = huffmanRoot;
    size_t pixelIndex = 0;

    // Decode the binary data using the Huffman Tree
    while (fread(&buffer, sizeof(unsigned char), 1, encodedFile) == 1) {
        for (int i = 7; i >= 0; i--) {
            int bit = (buffer >> i) & 1;
            currentNode = bit == 0 ? currentNode->left : currentNode->right;

            if (!currentNode->left && !currentNode->right) {
                unsigned int color = currentNode->color;
                decodedPixelData[pixelIndex++] = color & 0xFF;
                decodedPixelData[pixelIndex++] = (color >> 8) & 0xFF;
                decodedPixelData[pixelIndex++] = (color >> 16) & 0xFF;
                currentNode = huffmanRoot;

                if (pixelIndex >= row_padded * height) {
                    break;
                }
            }
        }
    }

    fclose(encodedFile);

    // Check if pixel data is complete
    if (pixelIndex != row_padded * height) {
        printf("Decoded data might be incomplete. Pixel data length: %zu\n", pixelIndex);
    } else {
        printf("Decoding completed successfully!\n");
    }

    // Open the output file for writing the BMP image
    const char *outputFileName = "decoded_image.bmp";
    FILE *outputFile = fopen(outputFileName, "wb");
    if (outputFile == NULL) {
        perror("Error creating output BMP file");
        free(decodedPixelData);
        return;
    }

    // Write the BMP header and info header from the original BMP file
    fwrite(bmpHeader, sizeof(unsigned char), headerSize, outputFile);

    // Write the decoded pixel data
    fwrite(decodedPixelData, sizeof(unsigned char), row_padded * height, outputFile);

    free(decodedPixelData);
    printf("Decoded image successfully written to %s\n", outputFileName);
    fclose(outputFile);
}
void getBmpDimensions(const char *filename, int *width, int *height, int *row_padded) {
    FILE *bmpFile = fopen(filename, "rb");
    if (!bmpFile) {
        perror("Error opening BMP file");
        return;
    }

    unsigned char bmpHeader[54];
    fread(bmpHeader, sizeof(unsigned char), 54, bmpFile);
    fclose(bmpFile);

    // Retrieve width and height from the BMP header
    *width = *(int*)&bmpHeader[18];
    *height = *(int*)&bmpHeader[22];

    // Calculate the padded row size
    int bytesPerPixel = 3; // 24-bit BMP
    int unpaddedRowSize = *width * bytesPerPixel;
    *row_padded = (unpaddedRowSize + 3) & (~3); // Round up to the nearest multiple of 4
}

int main() {
    const char *inputFileName = "sample.bmp";
    unsigned char *pixelData = NULL;
    unsigned int *colorFrequency = (unsigned int *)calloc(MAX_COLORS, sizeof(unsigned int));
    if (colorFrequency == NULL) {
        perror("Error allocating memory for color frequency array");
        return 1;
    }
    
    int width, height;

    // Read the BMP file
    readBMP(inputFileName, &pixelData, &width, &height, colorFrequency);

    // Create color frequency pairs
    ColorFrequencyPair *pairs;
    int size;
    pairs = createColorFrequencyPairs(colorFrequency, &size);
    if (pairs == NULL) {
        return 1;
    }

    // Build the Huffman tree
    HuffmanNode *huffmanRoot = NULL;
    buildHuffmanTree(pairs, size, &huffmanRoot);

    // Generate Huffman codes and print them
    char **codes = (char **)malloc(MAX_COLORS * sizeof(char *));
    char codeBuffer[256];
    generateHuffmanCodes(huffmanRoot, codes, codeBuffer, 0);

    // Encode the pixel data
    unsigned char *encodedData = NULL;
    size_t encodedSize = 0;
    encodePixelData(pixelData, width, height, (width * 3 + 3) & (~3), codes, &encodedData, &encodedSize);

    printf("Encoded data size (in bytes): %zu\n", encodedSize);

    const char *outputFileName = "encoded_output.bin"; // Change to desired file name

    // Call the write function with the encoded data
    writeEncodedDataToFile(outputFileName, encodedData, encodedSize);

    const char *encodedFileName = "encoded_output.bin";

    // Assuming these values are from the encoding stage
      // Replace with the actual Huffman tree from encoding
    int  row_padded ; // Replace with actual image width, height, and padded row size
    getBmpDimensions("sample.bmp", &width, &height, &row_padded);


    // Load BMP header from the original file
    FILE *originalBmpFile = fopen("sample.bmp", "rb");
    if (!originalBmpFile) {
        perror("Error opening original BMP file");
        return 1;
    }

    // Read BMP headers (file header + info header)
    unsigned char bmpHeader[54];
    fread(bmpHeader, sizeof(unsigned char), 54, originalBmpFile);
    fclose(originalBmpFile);

    // Call decode function with the BMP header information
    decodeBinaryFile(encodedFileName, huffmanRoot, width, height, row_padded, bmpHeader, 54);

    return 0;
}
    
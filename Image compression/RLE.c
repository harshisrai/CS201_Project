#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(1)

// BMP file header
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

// BMP info header
typedef struct {
    uint32_t size;
    int32_t width, height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPelsPerMeter, yPelsPerMeter;
    uint32_t clrUsed;
    uint32_t clrImportant;
} BMPInfoHeader;

#pragma pack()

#define RLE_THRESHOLD 5  // Adjusted threshold for selective RLE

// Check if three RGB values (pixels) are the same
int pixels_equal(uint8_t *pixel1, uint8_t *pixel2) {
    return (pixel1[0] == pixel2[0] && pixel1[1] == pixel2[1] && pixel1[2] == pixel2[2]);
}

// Selective RLE Compression
void selective_compress_rle(uint8_t *data, int size, FILE *outputFile) {
    int i = 0;
    while (i < size) {
        uint8_t *pixel = &data[i];
        int runLength = 1;

        // Calculate run length
        while (i + runLength * 3 < size && pixels_equal(&data[i], &data[i + runLength * 3]) && runLength < 255) {
            runLength++;
        }

        if (runLength >= RLE_THRESHOLD) {
            // Write compressed data for long runs
            fputc(runLength, outputFile);
            fwrite(pixel, 1, 3, outputFile);
        } else {
            // Write each pixel uncompressed for short runs
            for (int j = 0; j < runLength; j++) {
                fputc(1, outputFile);  // Run length of 1
                fwrite(&data[i + j * 3], 1, 3, outputFile);
            }
        }

        i += runLength * 3;
    }
}

// Decompress RLE data
void decompress_rle(FILE *inputFile, uint8_t *outputData, int dataSize) {
    int i = 0;

    while (i < dataSize) {
        int runLength = fgetc(inputFile);
        uint8_t pixel[3];
        fread(pixel, 1, 3, inputFile);

        for (int j = 0; j < runLength && i < dataSize; j++) {
            memcpy(&outputData[i], pixel, 3);
            i += 3;
        }
    }
}

// Compress BMP file with selective RLE
void compress_bmp(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    FILE *outputFile = fopen(outputPath, "wb");

    if (!inputFile || !outputFile) {
        perror("File error");
        return;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    // Read BMP headers
    fread(&header, sizeof(BMPHeader), 1, inputFile);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inputFile);

    // Write headers to output file
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    int dataSize = infoHeader.imageSize;
    uint8_t *data = (uint8_t*) malloc(dataSize);
    fread(data, 1, dataSize, inputFile);

    // Selectively compress pixel data and write to output file
    selective_compress_rle(data, dataSize, outputFile);

    free(data);
    fclose(inputFile);
    fclose(outputFile);
}

// Decompress BMP file
void decompress_bmp(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    FILE *outputFile = fopen(outputPath, "wb");

    if (!inputFile || !outputFile) {
        perror("File error");
        return;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    // Read headers from compressed file
    fread(&header, sizeof(BMPHeader), 1, inputFile);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inputFile);

    // Write headers to output BMP file
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);

    int dataSize = infoHeader.imageSize;
    uint8_t *data = (uint8_t*) malloc(dataSize);

    // Decompress the data
    decompress_rle(inputFile, data, dataSize);
    fwrite(data, 1, dataSize, outputFile);

    free(data);
    fclose(inputFile);
    fclose(outputFile);
}

// Main function to compress and decompress BMP files
int main() {
    const char *inputFile = "sample.bmp";
    const char *compressedFile = "compressed.rle";
    const char *decompressedFile = "decompressed.bmp";

    // Compress the BMP file
    compress_bmp(inputFile, compressedFile);
    printf("Selective compression completed: %s\n", compressedFile);

    // Decompress back to BMP
    decompress_bmp(compressedFile, decompressedFile);
    printf("Decompression completed: %s\n", decompressedFile);

    return 0;
}

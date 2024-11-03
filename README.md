# Text and Image Compression Project
## CS201_Project
This project implements text and image compression using multiple methods, including Huffman, Run-Length Encoding (RLE), and Lempel-Ziv-Welch (LZW) for text, and Huffman and RLE for images. Each method offers different approaches to achieve efficient compression and decompression of files. Below are detailed instructions for using each method.

## Table of Contents
- [Text Compression](#text-compression)
  - [Huffman Compression](#huffman-compression)
  - [RLE Compression](#rle-compression)
  - [LZW Compression](#lzw-compression)
  - [Testing Files](#testing-files)
    - [Text Files](#text-files)
    - [Custom Text Files](#custom-text-files)
- [Image Compression](#image-compression)
  - [Huffman Compression](#huffman-compression-image)
  - [RLE Compression](#rle-compression-image)
  - [Testing Files](#testing-files)
    - [Image Files](#image-files)  
    - [Custom Image Files](#custom-image-files)

---

## Text Compression

### Huffman Compression

1. **Sample File**: Ensure the sample text file is in the same directory as the code.
2. **Setup**: Edit the code to specify the sample file:
   ```c
   const char* inputFilename = "sample.txt";
   ```
3. **Execution**:
   - Compile and run the `.c` file.
4. **Output**:
   - A compressed `.bin` file.
   - A decompressed `.txt` file version of the `.bin` file.

### RLE Compression

1. **Sample File**: Place the sample file in the same directory.
2. **Setup**: Update the input filename in the code:
   ```c
   char *input = read_file("sample.txt", &input_length);
   ```
3. **Execution**:
   - Compile and run the `.c` file.
4. **Output**:
   - A compressed `.txt` file.
   - A decompressed `.txt` file version of the compressed output.

### LZW Compression

#### Compression
1. **Sample File**: Place the text file in the directory.
2. **Execution**:
   - Compile and run the `.c` file.
   - Input the file name when prompted.
3. **Output**: Generates `compressed.bin` as the compressed file.

#### Decompression
1. **Setup**: Place the `compressed.bin` file from the compression step in the directory.
2. **Execution**:
   - Compile and run the `.c` file.
3. **Output**: Generates `decompressed.txt` as the decompressed output.

## Testing Files

### Text Files
- The "Sample Text" folder contains sample text files for testing the text compression methods.

### Custom Text Files
- You are free to test any text file. Simply place the file in the directory and update the code as instructed for each compression method.

## Image Compression

### Huffman Compression <a name="huffman-compression-image"></a>

1. **Sample File**: Place the `.bmp` file in the directory.
2. **Setup**: Specify the image file in the code:
   ```c
   const char *inputFileName = "sample.bmp";
   ```
3. **Execution**:
   - Compile and run the `.c` file.
4. **Output**:
   - A `.bin` compressed file.
   - A decompressed `.bmp` file version of the `.bin` file.

### RLE Compression <a name="rle-compression-image"></a>

1. **Sample File**: Place the `.bmp` file in the directory.
2. **Setup**: Update the file name in the code:
   ```c
   const char *inputFile = "sample.bmp";
   ```
3. **Execution**:
   - Compile and run the `.c` file.
4. **Output**:
   - A `.rle` compressed file.
   - A decompressed `.bmp` file version of the `.rle` file.

## Testing Files

### Image Files
- The "Sample Image" folder contains `.bmp` images for testing the image compression methods.

### Custom Image Files
- For compatibility with the compression algorithms, images in formats such as `.png`, `.jpeg`, or `.bmp` can be standardized by opening the image in Microsoft Paint and saving it as a `.bmp` file. This ensures uniform formatting for processing with the compression code.
--- 

This README provides all the necessary information to run and test the compression methods. Adjust file names and paths as needed, and enjoy exploring the compression techniques!

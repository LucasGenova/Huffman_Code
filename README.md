# Huffman Coding

Huffman code implementation capable of encoding text and images.

## Running the code

- Clone the project.
- On the base of the project, run:
```bash
$ gcc -o huff main.c huffman.c bitfile.c
$ ./huff

```

## Use

On the application, type:

- 1: for encoding a file, and creating a .huf file.
- 2: for decoding a .huf file.
- 0: for exiting the application.

## Additional information

A .bmp file is provided, and has been successfully encoded and decoded.

Decoded images need to renamed with the appropriate file extension. The .huf file does not carry this information.
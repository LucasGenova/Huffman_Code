#ifndef _BITFILE_H
#define _BITFILE_H

typedef struct BitFile{
    int EOBF;

    FILE* fp;
    __uint32_t buffer;
    int bit_count;

    int bits_writen;
    int bit_filling;
}BIT_FILE;

typedef struct Code{
    __uint32_t code;
    size_t len;
}T_code;

//Bitfiles functions prototypes:

//Opens a Bitfile with the specified mode
BIT_FILE* open_bit_file(char* filename, char* mode);

//Reads of a code of the specified size from the specified bitfile
void read_code(int size, T_code* code, BIT_FILE* input_file);

//writes a code into a Bitfile
void write_code(T_code* code, BIT_FILE* bit_file);

//Flushes the buffer and bit parity of a Bitfile
void flush_bit_file(BIT_FILE* bit_file);

//Closes a Bitfile
void close_bit_file(BIT_FILE* bit_file);

#endif
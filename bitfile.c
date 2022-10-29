#include <stdio.h>
#include <stdlib.h>
#include "bitfile.h"

//Bitfile functions implementations:

BIT_FILE* open_bit_file(char* filename, char* mode ){
    BIT_FILE* bit_file = (BIT_FILE*) malloc(sizeof(BIT_FILE));

    bit_file->EOBF=0;
    bit_file->fp = fopen(filename, mode);
 //oi   
    bit_file->buffer=0;
    bit_file->bit_count=0;
    bit_file->bits_writen=0;
    bit_file->bit_filling=0;

    return bit_file;
}

void read_code(int len, T_code* code, BIT_FILE* input_file){
     __uint8_t bits=0;
    int frs=0;

    //Not enough bits in the buffer. Reads from file and stores in buffer
    if((input_file->bit_count-input_file->bit_filling)<len){
        frs = fread(&bits, 1, 1, input_file->fp);

        if(!frs && !input_file->EOBF){
            input_file->EOBF=1;
            input_file->buffer>>=input_file->bit_filling;
            input_file->bit_count-=input_file->bit_filling;
            input_file->bit_filling=0;
        }
            
        input_file->buffer<<=(frs*8);
        input_file->buffer|=bits;
        input_file->bit_count+=(frs*8);
    }

    //Manages to get all needed bits
    if((input_file->bit_count - input_file->bit_filling)>=len){
        code->code=input_file->buffer>>(input_file->bit_count- len);
        code->len=len;

        input_file->bit_count-=len;
        input_file->buffer&=~((~0)<<(input_file->bit_count));
    }
    else{//File ended without the needed bits
        code->code=(input_file->buffer>>input_file->bit_filling);
        code->len=(input_file->bit_count-input_file->bit_filling);

        input_file->buffer=0;
        input_file->bit_count=0;
    }
}

void write_code(T_code* code, BIT_FILE* bit_file){

    //Writes in buffer
    bit_file->buffer<<=(code->len);
    bit_file->buffer|=(code->code);
    bit_file->bit_count+=(code->len);

    //Flushes if possible
    while(bit_file->bit_count>=8){
        fprintf(bit_file->fp, "%c", ((bit_file->buffer>>(bit_file->bit_count-8))&0xFF));
        bit_file->bit_count-=8;
        bit_file->buffer&=(~((~0)<<(bit_file->bit_count)));

        bit_file->bits_writen+=8;
    }
}

void flush_bit_file(BIT_FILE* bit_file){
    T_code code, first_byte;

    //Writes buffer in file with 0 padding
    code.code=0;
    code.len=((8-(bit_file->bit_count))%8);

    write_code(&code, bit_file);

    //Gets first byte
    fflush(bit_file->fp);
    fseek(bit_file->fp,0, SEEK_SET);
    read_code(8, &first_byte, bit_file);

    //Extract first 3 bits (bit parity)
    first_byte.code&=(~((0x07)<<5));
    first_byte.code|=((code.len)<<5);

    //Writes bit parity down
    fseek(bit_file->fp, 0, SEEK_SET);
    write_code(&first_byte, bit_file);
    fseek(bit_file->fp, 0, SEEK_END);
}

void close_bit_file(BIT_FILE* bit_file){
    flush_bit_file(bit_file);
    fflush(bit_file->fp);
    fclose(bit_file->fp);
}

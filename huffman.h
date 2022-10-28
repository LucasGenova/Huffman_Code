#ifndef _HUFFMAN_H
#define _HUFFMAN_H

typedef struct Node{
    struct Node* ln;
    struct Node* rn;

    //node info
}T_node;

typedef struct Elem{
    T_node* tree;
    struct Elem* next;
}T_elem;

typedef struct List{
    T_elem* head;
    int len;
}T_list;

//List Implementation functions:


//Huffman Encoder/Decoder implementation functions:
int compress(FILE* input_file);
int decompress(FILE* input_file);

#endif
#ifndef _HUFFMAN_H
#define _HUFFMAN_H

#define INIT_LIST(list); list.head=NULL;list.len=0;

typedef struct Node{
    struct Node* ln;
    struct Node* rn;

    __uint8_t c;
    int weight;
}T_node;

typedef struct Elem{
    T_node* tree;
    struct Elem* next;
}T_elem;

typedef struct List{
    T_elem* head;
    int len;
}T_list;

typedef struct Code{
    __uint32_t code;
    size_t len;
}T_code;

//List/Tree functions prototypes:

//Creates a new node for the character c with specified weight
T_node* frequency_to_node(__uint8_t c, int frequency);

//Adds a node to a list in increasing order based on the node weight
void add_node_inc(T_list* list, T_node* node); 

//Returns the head of the list
T_elem* remove_root(T_list* list);

//Removes the elem structure that encapsulate the node
T_node* unpack(T_elem* elem);

//Creates a new tree with the specified branches
T_node* merge_tree(T_node* ln, T_node* rn);

//Huffman Encoder/Decoder functions prototypes:

//Generates a T_code conversion array base on the specified tree
void generate_conv_table(T_node* tree, T_code* conv_table, T_code cur_code);

int compress(FILE** input_file);
int decompress(FILE** input_file);

#endif
#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

//List/Tree functions implementations:

//generates a new node
T_node* frequency_to_node(__uint8_t c, int frequency){
    T_node* node = (T_node*) malloc(sizeof(T_node));

    node->ln = NULL;
    node->rn = NULL;

    node->c = c;
    node->weight = frequency;

    return node;
}

void add_node_inc(T_list* list, T_node* node){
    T_elem *elem = (T_elem*) malloc(sizeof(T_elem));

    elem->tree = node;
    elem->next = NULL;

    list->len++;

    if(!list->head){
        list->head=elem;
        return;
    }

    if(list->head->tree->weight > elem->tree->weight){
        elem->next = list->head;
        list->head = elem;
        return;
    }
        
    T_elem* aux = list->head;
    while(aux->next && aux->next->tree->weight < elem->tree->weight)
        aux=aux->next;

    elem->next=aux->next;
    aux->next=elem;
}

T_elem* remove_root(T_list* list){
    T_elem* elem = list->head;
    list->head = list->head->next;
    list->len--;

    return elem;
}

T_node* unpack(T_elem* elem){
    T_node* node = elem->tree;
    free(elem);
    return node;
}

T_node* merge_tree(T_node* ln, T_node* rn){
    T_node* new_tree = (T_node*) malloc(sizeof(T_node));

    new_tree->ln = ln;
    new_tree->rn = rn;
    new_tree->c = '-';
    new_tree->weight = ln->weight + rn->weight;

    return new_tree;
}



//Huffman functions implementations:
void generate_conv_table(T_node* tree, T_code* conv_table, T_code cur_code){
    if(!tree->ln && !tree->rn){
        cur_code.len++;
        
        (conv_table[tree->c]).code = cur_code.code;
        (conv_table[tree->c]).len = cur_code.len;
    }

    if(tree->ln){
        cur_code.code<<=1;
        generate_conv_table(tree->ln, conv_table, cur_code);
        cur_code.code>>=1;
    }

    if(tree->ln){
        cur_code.code<<=1;
        cur_code.code|=1;
        generate_conv_table(tree->rn, conv_table, cur_code);
        cur_code.code>>=1;
    }

}

int compress(FILE** input_file){
    int i, c[256]={0};
    char read;

    T_list nodes;
    INIT_LIST(nodes);

    T_code conv_table[256]={0}, code={0,0};

    //get frequencies
    while(fread(&read, 1, 1, (*input_file)))
        c[read]++;

    //get tree
    //generate list of nodes
    for(i=0; i<256; i++)
        if(c[i])
            add_node_inc(&nodes, frequency_to_node(i,c[i]));

    //merge tree
    while(nodes.len>1)
        add_node_inc(&nodes, merge_tree(unpack(remove_root(&nodes)), unpack(remove_root(&nodes))));

    //get conversion table
    generate_conv_table(nodes.head->tree, conv_table, code);

    //run file back and translate into new file
    //buffer bit parity
    //write tree
    

    return 1;
}

int decompress(FILE** input_file){
    //save bit parity
    
    //retrieve tree

    //translate file

    return 1;
}
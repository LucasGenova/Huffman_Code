#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

char HUFF_ERROR[100];

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
    cur_code.len++;

    if(!tree->ln && !tree->rn){        
        (conv_table[tree->c]).code = cur_code.code;
        (conv_table[tree->c]).len = cur_code.len-1;
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

void write_tree(T_node* tree, BIT_FILE* output_file){
    T_code code;

    if(!tree)
        return;
    
    if(!tree->ln && !tree->rn){
        code.code=1;
        code.len=1;
        write_code(&code, output_file);
        
        code.code=tree->c;
        code.len=8;
        write_code(&code, output_file);
    }
    else{
        code.code=0;
        code.len=1;
        write_code(&code, output_file);
    }

    write_tree(tree->ln, output_file);
    write_tree(tree->rn, output_file);
}

void read_tree(T_node** tree, BIT_FILE* input_file){
    T_code code;

    read_code(1, &code, input_file);

    (*tree) = frequency_to_node(0, 0);

    if(!code.code){
        read_tree(&((*tree)->ln), input_file);
        read_tree(&((*tree)->rn), input_file);
    }
    else{
        read_code(8, &code, input_file);
        (*tree)->c=code.code;
        (*tree)->weight=0;
        (*tree)->ln=NULL;
        (*tree)->rn=NULL;
    }
}

T_node* huf_read(BIT_FILE* input_file, T_node* tree){
    if(!tree->ln && !tree->rn)
        return tree;

    T_code code;

    read_code(1, &code, input_file);

    return huf_read(input_file, ((!code.code)?tree->ln:tree->rn));
}

int compress(char *filename){
    FILE *input_file = fopen(filename, "r");

    if(!input_file){
        strcpy(HUFF_ERROR, "O arquivo não pode ser aberto.\n");
        return 0;
    }  

    int i, c[256]={0};
    char read;

    T_list nodes;
    INIT_LIST(nodes);

    T_code conv_table[256]={0}, code={0,0};

    //get frequencies
    while(fread(&read, 1, 1, input_file))
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

    //Gets output filename and opens bitfile
    char output_filename[100]="";
    strcpy(output_filename, filename);
    
    for(i=0; output_filename[i] && output_filename[i]!='.'; i++);
    output_filename[i]='\0';

    strcat(output_filename, ".huf");

    BIT_FILE* output_file = open_bit_file(output_filename, "w+");

    //buffer bit parity
    code.code=0;
    code.len=3;
    write_code(&code, output_file);

    //write tree
    write_tree(nodes.head->tree, output_file);

    //run file back and translate into new file
    rewind(input_file);
    while(fread(&read, 1, 1, input_file))
    write_code(&conv_table[read], output_file);
    
    close_bit_file(output_file);
    fclose(input_file);

    return 1;
}

int decompress(char *filename){
    //Opens bitfile
    BIT_FILE* huff_file =  open_bit_file(filename, "r");

    if(!huff_file->fp){
        strcpy(HUFF_ERROR, "O arquivo não pode ser aberto.\n");
        return 0;
    }

    int i;

    T_code code;
    T_node* r_tree, *huff_code;

    //save bit parity
    read_code(3, &code, huff_file);
    huff_file->bit_filling=code.code;
    
    //retrieve tree
    read_tree(&r_tree, huff_file);

    //Gets output filename and opens bitfile
    char output_filename[100]="";
    strcpy(output_filename, filename);
    
    for(i=0; output_filename[i] && output_filename[i]!='.'; i++);
    output_filename[i]='\0';

    strcat(output_filename, "_decoded.txt");

    FILE* output_file = fopen(output_filename, "w+");

    //translate file
    while(!huff_file->EOBF | huff_file->buffer){
        huff_code = huf_read(huff_file, r_tree);

        if(!huff_file->EOBF | huff_file->buffer)
            fwrite(&huff_code->c, 1, 1, output_file);
    }

    fclose(output_file);

    return 1;
}
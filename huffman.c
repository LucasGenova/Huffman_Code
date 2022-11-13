#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

#define PADDED_APPEND(S, n, s) sprintf(S, "%s%-*s",S, n, s);
#define ADD_FORMATED_SIZE(S, bits, p1, p2, dp) sprintf(S,"%s%*d bits ~ %*.*lf bytes", S, p1, bits, p2+3, dp, bits/8.0);
#define ADD_FORMATED_SIZE_SGND(S, bits, p1, p2, dp) sprintf(S,"%s%+*d bits ~ %+*.*lf bytes", S, p1, -bits, p2+3, dp, -bits/8.0);
#define ADD_PERCENTAGE(S, tt, ca, dp) sprintf(S,"%s | %+3.*lf%%", S, (-100.0*ca)/tt, dp);

//Global variables and flags:
char HUFF_ERROR[100];
char HUFF_RESULT[3000]="\0";
int IBC=-1;

int D_INFO=1;
//Human readable header?

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

    int i, c[256]={0}, padding_bc, tree_bc, encoding_bc;
    char read;

    T_list nodes;
    INIT_LIST(nodes);

    T_code conv_table[256]={0}, code={0,0};

    //get frequencies
    IBC=-1;
     while(!feof(input_file)){
        c[fgetc(input_file)]++;
        IBC++;
    }

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
    tree_bc=(output_file->bits_writen + output_file->bit_count)-3;

    //run file back and translate into new file
    rewind(input_file);
    while(!feof(input_file))
        write_code(&conv_table[fgetc(input_file)], output_file);
    
    encoding_bc=(output_file->bits_writen + output_file->bit_count)-(tree_bc+3);
    
    padding_bc=8-output_file->bit_count;
    close_bit_file(output_file);
    fclose(input_file);

    if(D_INFO){
        int p1 = snprintf(NULL, 0, "%i", (8*IBC))+1, p2 = snprintf(NULL, 0, "%i", IBC)+2;
        HUFF_RESULT[0]='\0';

        PADDED_APPEND(HUFF_RESULT, 32, "\nTamanho original:");
        ADD_FORMATED_SIZE(HUFF_RESULT, (8*IBC), p1, p2, 0);

        PADDED_APPEND(HUFF_RESULT, 33, "\n\nTamanho comprimido:");
        ADD_FORMATED_SIZE(HUFF_RESULT, (padding_bc+3+tree_bc+encoding_bc), p1, p2, 0);

        PADDED_APPEND(HUFF_RESULT, 32, "\n    Byte parity (3) + Padding:");
        ADD_FORMATED_SIZE(HUFF_RESULT, (padding_bc+3), p1, p2, 3);

        PADDED_APPEND(HUFF_RESULT, 32, "\n    Arvore:");
        ADD_FORMATED_SIZE(HUFF_RESULT, tree_bc, p1, p2, 3);

        PADDED_APPEND(HUFF_RESULT, 32, "\n    Texto comprimido:");
        ADD_FORMATED_SIZE(HUFF_RESULT, encoding_bc, p1, p2, 3);

        PADDED_APPEND(HUFF_RESULT, 33, "\n\nRedução do arquivo:");

        PADDED_APPEND(HUFF_RESULT, 32, "\n    Texto + Metadados:");
        ADD_FORMATED_SIZE_SGND(HUFF_RESULT, ((8*IBC)-(padding_bc+3+tree_bc+encoding_bc)), p1, p2, 3);
        ADD_PERCENTAGE(HUFF_RESULT, (8*IBC), ((8*IBC)-(padding_bc+3+tree_bc+encoding_bc)), 3);

        PADDED_APPEND(HUFF_RESULT, 32, "\n    Texto (sem metadados):");
        ADD_FORMATED_SIZE_SGND(HUFF_RESULT, ((8*IBC)-encoding_bc), p1, p2, 3);
        ADD_PERCENTAGE(HUFF_RESULT, (8*IBC), ((8*IBC)-encoding_bc), 3);

        PADDED_APPEND(HUFF_RESULT, 32, "\n    Texto + Padding (sem meta):");
        ADD_FORMATED_SIZE_SGND(HUFF_RESULT, ((8*IBC)-(encoding_bc+padding_bc)), p1, p2, 3);
        ADD_PERCENTAGE(HUFF_RESULT, (8*IBC), ((8*IBC)-(encoding_bc+padding_bc)), 3);
    }

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
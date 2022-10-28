#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"
#include "bitfile.h"

#define CLR_BFFR(stream); while(fgetc(stream)!='\n');

int main(int argc, char** argv){
    if(argc==1){
        int RUNNING=1;
        char op='-', user_input[51];
        FILE* input_file;

        do{
            printf("[1] Comprimir arquivo\n[2] Decomprimir arquivo\n[0] Sair\n\n");
            scanf("%c", &op);
            CLR_BFFR(stdin);

            switch(op){
                case '1': //Compress
                    printf("Digite o nome do arquivo a ser comprimido: ");
                    scanf("%50s", user_input);
                    CLR_BFFR(stdin);

                    input_file = fopen(user_input, "r");

                    if(input_file){
                        compress(input_file);

                        fclose(input_file);
                    }
                    else{
                        printf("\nO arquivo não pode ser aberto.\n");
                    }
                    break;

                case '2': //Decompress
                    printf("Digite o nome do arquivo a ser decomprimido: ");
                    scanf("%50s", user_input);
                    CLR_BFFR(stdin);

                    input_file = fopen(user_input, "r");

                    if(input_file){
                        decompress(input_file);

                        fclose(input_file);
                    }
                    else{
                        printf("\nO arquivo não pode ser aberto.\n");
                    }
                    break;

                case '0':
                    RUNNING=0;
                    break;
                
                default:
                    printf("\nOpção inválida!\n");
            }

        }while(RUNNING);
        
    }


    return 0;
}
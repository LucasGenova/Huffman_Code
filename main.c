#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "bitfile.h"

#define CLR_BFFR(stream); while(fgetc(stream)!='\n');

char APP_ERROR[500];

int main(int argc, char** argv){
    if(argc==1){
        int RUNNING=1;
        char op='-', filename[51];

        do{
            

            if(HUFF_RESULT[0]!='\0'){
                system("clear");
                printf(HUFF_RESULT);
                HUFF_RESULT[0]='\0';
                
                printf("\n\nAperte enter pra continuar...");
                while(getchar()!='\n');
            }

            system("clear");
            printf(APP_ERROR);
            APP_ERROR[0]='\0';

            printf("[1] Comprimir arquivo\n[2] Decomprimir arquivo\n[0] Sair\n\n");

            scanf("%c", &op);
            CLR_BFFR(stdin);

            switch(op){
                case '1': //Compress
                    printf("Digite o nome do arquivo a ser comprimido: ");
                    scanf("%50s", filename);
                    CLR_BFFR(stdin);

                    if(!compress(filename)){
                        strcpy(APP_ERROR, "Falha na compressão:\n");
                        strcat(APP_ERROR, HUFF_ERROR);
                        strcat(APP_ERROR, "\n");
                    }
                        
                    break;

                case '2': //Decompress
                    printf("Digite o nome do arquivo a ser decomprimido: ");
                    scanf("%50s", filename);
                    CLR_BFFR(stdin);

                    if(!decompress(filename)){
                        strcpy(APP_ERROR, "Falha na decompressão:\n");
                        strcat(APP_ERROR, HUFF_ERROR);
                        strcat(APP_ERROR, "\n");
                    }
                        
                    break;

                case '0':
                    RUNNING=0;
                    break;
                
                default:
                    strcpy(APP_ERROR, "Opção inválida.\n");
            }

        }while(RUNNING);
    }

    //Todo: Add terminal app functionality

    return 0;
}
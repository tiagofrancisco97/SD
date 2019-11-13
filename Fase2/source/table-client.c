/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<signal.h>

#include "client_stub.h"
#include "network_client.h"

int main(int argc, char **argv){

    if(argc != 2){
        printf("Deve introduzir: table-client <ip servidor>:<porta servidor>\n");
        printf("Exemplo de uso: ./binary/table-client 127.0.0.1:50000\n");
        return -1;
    }
    printf("Antes de estabelecer a ligação ao servidor.\n");

    struct rtable_t *rtable = rtable_connect(argv[1]);
    if (rtable == NULL){
        rtable_disconnect(rtable);
        return -1;
    }

    printf("Após estabelecer a ligação ao servidor.\n");


    char *token = "";

    char buf[100];

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);

    while(strcmp(token, "quit") != 0){
        printf("Insira o comando pretendido:\nsize\ndel <key>\nget <key>\nput <key> <data>\ngetkeys\nquit\n");
        fgets(buf, 100, stdin);
        buf[strlen(buf) - 1] = '\0';
        token = strtok(buf, " ");

        if(strcmp(token,"size") == 0){
            printf("Numero de elementos na tabela: %d\n", rtable_size(rtable));
            token = "";
        }

        if(strcmp(token, "del") == 0){
            token = strtok(NULL, " ");
            char* key = strdup(token);
            int i = rtable_del(rtable, key);
            free(key);
            if(i == 0){
                printf("Elemento removido\n");
            }else{
                printf("Chave nao encontrada\n");
            }
            token = "";
        }

        if(strcmp(token, "get") == 0){
            token = strtok(NULL, " ");
            char* key = strdup(token);
            printf("Procura pela key %s\n", key);
            struct data_t *data = rtable_get(rtable, key);
            free(key);
            printf("value: %s\n", data->data);
            if (strcmp(data->data, "") == 0) {
                free(data);
            } else {
                data_destroy(data);
            }
            token = "";
        }

        if(strcmp(token, "put") == 0){
            token = strtok(NULL, " ");
            char* key = strdup(token);
            token = strtok(NULL, " ");

            if(token == NULL){
                printf("Escreva um comando valido:\nput <key> <data>\n");
                free(key);
            }else{
                struct data_t *data = data_create2(strlen(token) + 1, token);
                struct entry_t *entry = entry_create(key, data);

                int i = rtable_put(rtable, entry);

                //o free_message_t no stub da free das strings que forma o entry e o data, so preciso de dar free dos seus ponteiros
                free(entry);
                free(data);
                free(key);

                if(i == 0){
                    printf("Elemento introduzido\n");
                }else{
                    printf("Elemento nao introduzido\n");
                }
                token = "";
            }
        }

        if(strcmp(token, "getkeys") == 0){
            char **keys = rtable_get_keys(rtable);
            for (int i = 0; keys[i] != NULL; i++){
                printf("%s \n", keys[i]);
            }
            rtable_free_keys(keys);
            token = "";
        }

    }

    printf("A desconectar\n");
    network_close(rtable);
    return rtable_disconnect(rtable);
}

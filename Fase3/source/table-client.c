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
        printf("Insira o comando pretendido:\nsize\ndel <key>\nget <key>\nput <key> <data>\ngetkeys\nverify <n_op>\nquit\n");
        fgets(buf, 100, stdin);
        buf[strlen(buf) - 1] = '\0';
        token = strtok(buf, " ");

        if(strcmp(token,"size") == 0){
            printf("Numero de elementos na tabela: %d\n", rtable_size(rtable));
            token = "";
        }

        if(strcmp(token, "del") == 0){
            token = strtok(NULL, " ");
            char *key = NULL;
            if (token != NULL) {
                key = strdup(token);
            }
            if (key != NULL) {
                int i = rtable_del(rtable, key);
                if (i == 0) {
                    printf("Elemento removido\n");
                } else {
                    printf("Chave nao encontrada\n");
                }
            } else {
                printf("Escreva um comando valido:\ndel <key>\n");
            }
            free(key);
            token = "";
        }

        if(strcmp(token, "get") == 0){
            token = strtok(NULL, " ");
            char* key = NULL;
            if (token != NULL){
                key = strdup(token);
            }
            if (key != NULL) {
                printf("Procura pela key %s\n", key);
                struct data_t *data = rtable_get(rtable, key);
                printf("value: %s\n", data->data);
                if (strcmp(data->data, "") == 0) {
                    free(data);
                } else {
                    data_destroy(data);
                }
            } else {
                printf("Escreva um comando valido:\nget <key>\n");
            }
            free(key);
            token = "";
        }

        if(strcmp(token, "put") == 0){
            token = strtok(NULL, " ");
            char *key = NULL;
            if (token != NULL){
                key = strdup(token);
            }
            token = strtok(NULL, " ");
            char *dados = NULL;
            if (token != NULL){
                dados = strdup(token);
            }

            if(key == NULL || dados == NULL){
                printf("Escreva um comando valido:\nput <key> <data>\n");
            }else{
                struct data_t *data = data_create2(strlen(dados) + 1, dados);
                struct entry_t *entry = entry_create(key, data);

                int i = rtable_put(rtable, entry);

                //o free_message_t no network client da free das strings que forma o entry e o data, so preciso de dar free dos seus ponteiros
                free(entry);
                free(data);

                if(i != -1){
                    printf("Operação com codigo %d\n", i);
                }else{
                    printf("Erro na operação\n");
                }
            }
            free(dados);
            free(key);
            token = "";
        }

        if(strcmp(token, "getkeys") == 0){
            char **keys = rtable_get_keys(rtable);
            if (keys != NULL) {
                for (int i = 0; keys[i] != NULL; i++) {
                    printf("%s \n", keys[i]);
                }
                rtable_free_keys(keys);
            } else {
                printf("A tabela não tem elementos\n");
            }
            token = "";
        }

        if(strcmp(token, "verify") == 0){
            token = strtok(NULL, " ");
            char *op = NULL;
            if (token != NULL) {
                 op = strdup(token);
            }
            if(op == NULL){
                printf("Escreva um comando valido:\nverify <n_op>\n");
            } else {
                int boolean = rtable_verify(rtable, atoi(op));
                if (boolean == 0) {
                    printf("A operação já foi realizada\n");
                } else {
                    printf("A operação ainda não foi realizada\n");
                }
            }
            free(op);
            token = "";
        }

    }

    printf("A desconectar\n");
    network_close(rtable);
    return rtable_disconnect(rtable);
}

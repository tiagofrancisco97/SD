/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */



#include "table_skel.h"
#include "message.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct table_t *tabela;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists){

    if(n_lists < 1){
        return -1;
    }
    tabela = table_create(n_lists);
    if(tabela == NULL){
        return -1;
    }
    return 0;
}

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy(){
    if(tabela != NULL){
        table_destroy(tabela);
    }
}

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(struct message_t *msg){

    if(msg == NULL){
        return -1;
    }
    if(msg->opcode < 0 || msg->opcode > 99 ||msg->c_type < 0 || msg->c_type > 60){
        return -1;
    }

    int result;
    struct data_t *data;
    char **keys;

    switch (msg->opcode){
        case MESSAGE_T__OPCODE__OP_SIZE:
            msg->opcode=MESSAGE_T__OPCODE__OP_SIZE+1;
            msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
            msg->data_size = table_size(tabela);
            return 0;

        case MESSAGE_T__OPCODE__OP_DEL:
            result = table_del(tabela, msg->key);
            if(result == 0 ){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            }
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return 0;

        case MESSAGE_T__OPCODE__OP_GET:
            data = table_get(tabela, msg->key);
            msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            if(data != NULL){
                msg->data_size = data->datasize;
                msg->data = data->data;
            } else {
                msg->data_size = 0;
                msg->data = NULL;
            }
            free(data);
            return 0;

        case MESSAGE_T__OPCODE__OP_PUT:
            data = data_create2(msg->data_size, msg->data);
            result = table_put(tabela, msg->key, data);
            free(data);
            if(result == 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT+1;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            }
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return 0;

        case MESSAGE_T__OPCODE__OP_GETKEYS:
            keys= table_get_keys(tabela);
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

            int size = table_size(tabela);
            msg->n_keys = size;
            msg->keys = malloc(sizeof(char*) * size);
            for (int i = 0; i < size; i++){
                msg->keys[i] = (keys[i]);
            }
            return 0;
    }
    return -1;
}
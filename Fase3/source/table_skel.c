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
#include "pthread.h"

struct table_t *tabela;

int last_assigned = 0;
int op_count = -1;
struct task_t *queue_head;
pthread_mutex_t queue_lock, table_lock;
pthread_cond_t queue_not_empty;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists){
    pthread_t thread;
    //queue_head=NULL;
    if(n_lists < 1){
        return -1;
    }
    tabela = table_create(n_lists);
    if(tabela == NULL){
        return -1;
    }

    /* criação de nova thread */
	if (pthread_create(&thread, NULL, &process_task, NULL) != 0){
		perror("\nThread não criada.\n");
		exit(EXIT_FAILURE);
	}
    pthread_mutex_init(&queue_lock, NULL);
    pthread_mutex_init(&table_lock, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
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
            //data = data_create2(msg->data_size, msg->data);
            result=insereTask(0,msg->key,NULL);

            if(result != -1 ){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
                msg->data_size = result;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }
            /*result = table_del(tabela, msg->key);
            if(result == 0 ){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }*/
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
            //data = data_create2(msg->data_size, msg->data);
            result=insereTask(1,msg->key,msg->data);
            if(result != -1 ){
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
                msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
                msg->data_size = result;
                //pthread_cond_signal(&queue_not_empty);
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }

            /*result = table_put(tabela, msg->key, data);
            free(data);
            if(result == 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT+1;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            }
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;*/
            return 0;

        case MESSAGE_T__OPCODE__OP_GETKEYS:
            keys = table_get_keys(tabela);
            msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS+1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

            int size = table_size(tabela);
            msg->n_keys = size;
            if (size > 0) {
                msg->keys = malloc(sizeof(char *) * size);
                for (int i = 0; i < size; i++) {
                    msg->keys[i] = strdup(keys[i]);
                }
            }
            table_free_keys(keys);
            return 0;

        case MESSAGE_T__OPCODE__OP_VERIFY:
            result = verify(msg->data_size);
            if (result == 0){
                msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            }
            return 0;
    }
    return -1;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n){
    //O quer dizer operaçao realizda
    //1 quer dizer que nao foi realizada
    return op_n<=op_count? 0 : 1;
}

/* Função do thread secundário que vai processar pedidos de escrita.
*/
void * process_task (void *params){
    while(1){
        pthread_mutex_lock(&queue_lock); 
        while(queue_head==NULL){
            pthread_cond_wait(&queue_not_empty,&queue_lock);

        }
        if(queue_head!=NULL){

            struct task_t *atual = queue_head;
	    struct task_t *head = queue_head;
            while(atual!=NULL){
                if(atual->op==0){//del
                    if(atual->key!=NULL){
                        pthread_mutex_lock(&table_lock);
                        table_del(tabela, atual->key);
                        pthread_mutex_unlock(&table_lock);
                        op_count++;
                    }
                }else{ //insert
                    if(atual->key!=NULL && atual->data!=NULL ){
                        //printf("A inserir key %s com value %s\n", atual->key, atual->data);
                        pthread_mutex_lock(&table_lock);
                        table_put(tabela, atual->key, data_create2(strlen(atual->data)+1, atual->data));
                        pthread_mutex_unlock(&table_lock);
                        op_count++;
                    }
                }
                atual=atual->next;
            }
            queue_head=atual;
            pthread_mutex_unlock(&queue_lock);
        }


    }
}

/*Insere uma task na queue
 * Retorna o numero da op da task em caso de sucesso, -1 em caso de erro
 */
int insereTask(int op,char* key, char *data){
    if(key==NULL){
        return -1;
    }
    struct task_t* t=(struct task_t*) malloc(sizeof(*t));
    t->op_n=last_assigned;
    t->key=malloc(strlen(key)+1);
    t->next=NULL;
    strcpy(t->key,key);
    if(op==0){//delete
        t->op=0;
    }else{//insert
        if(data==NULL){
            return -1;
        }
        t->op=1;
        t->data=malloc(strlen(data)+1);
        strcpy(t->data,data);
        //printf("inserindo no taks value %s \n", t->data);
    }
    pthread_mutex_lock(&queue_lock);
    if(queue_head==NULL){
            queue_head=t;
            t->next=NULL;
            //printf("inserindo no taks key %s \n", queue_head->key);

    } else{ 
        struct task_t *atual=queue_head;
        while(atual->next!=NULL){
            atual=atual->next;
        }
        atual->next=t;
        t->next=NULL;
        //printf("inserindo no taks key %s \n", atual->next->key);

    }
    last_assigned++;
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);
    return t->op_n;
}
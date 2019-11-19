/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include "client_stub.h"
#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdmessage.pb-c.h"
#include "network_client.h"
#include "message.h"

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtable_t *rtable_connect(const char *address_port){

    if(address_port == NULL){
        return NULL;
    }

    struct rtable_t *remote_table = (struct rtable_t*) malloc(sizeof(struct rtable_t));

    char *address_copy = (char *) malloc(strlen(address_port) + 1);
    if (address_copy == NULL){
        free(remote_table);
        return NULL;
    }

    // get IP address and port
    strcpy(address_copy, address_port);
    char *address = strtok(address_copy, ":");
    char *port = strtok(NULL, ":");

    // create TCP socket
    if ((remote_table->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        free(address);
        free(remote_table);
        fprintf(stderr, "Erro ao criar socket.");
        return NULL;
    }

    // Preenche estrutura server para estabelecer conexão
    remote_table->address.sin_family = AF_INET;
    remote_table->address.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, address, &remote_table->address.sin_addr) < 1) { // Endereço IP
        free(address);
        free(remote_table);
        printf("Erro ao converter IP\n");
        network_close(remote_table);
        return NULL;
    }

    // connect to server
    if (network_connect(remote_table) < 0) {
        free(address);
        free(remote_table);
        fprintf(stderr, "Erro ao ligar ao servidor.\n");
        return NULL;
    }

    free(address);
    return remote_table;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable){
    if (rtable == NULL){
        return -1;
    }
    printf("Vai dar free da rtable\n");
    free(rtable);
    return 0;
}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry){
    if (rtable == NULL || entry == NULL){
        printf("Deu null");
        return -1;
    }
    MessageT request;

    message_t__init(&request);

    request.opcode = MESSAGE_T__OPCODE__OP_PUT;
    request.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    request.data_size = entry->value->datasize;
    request.data = entry->value->data;
    request.key = entry->key;

    request.keys = NULL;
    request.n_keys = 0;

    if (request.data == NULL || request.key == NULL){
        return -1;
    }

    struct message_t *req = MTom(&request);
    if (req == NULL || req->data == NULL || req->key == NULL){
        return -1;
    }

    struct message_t *response = network_send_receive(rtable, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return -1;
    }
    int r=response->data_size;

    free_message_t(response);
    return r;
}

/* Função para obter um elemento da tabela.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtable_get(struct rtable_t *rtable, char *key){
    if (rtable == NULL || key == NULL){
        return NULL;
    }
    struct _MessageT request;

    message_t__init(&request);

    request.opcode = MESSAGE_T__OPCODE__OP_GET;
    request.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    request.key = key;

    request.data = NULL;
    request.data_size = 0;
    request.keys = NULL;
    request.n_keys = 0;

    if(request.key == NULL){
        return NULL;
    }

    struct message_t *req = MTom(&request);
    if (req == NULL || req->key == NULL){
        return NULL;
    }

    struct message_t *response = network_send_receive(rtable, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return NULL;
    }
    printf("O tamanho retornado eh %d\n", response->data_size);
    if (response->data_size > 0){
        struct data_t *data = data_create2(response->data_size, response->data);
        struct data_t *dup = data_dup(data);
        free_message_t(response);
        free(data);
        return dup;
    } else {
        free_message_t(response);
        return data_create2(1, "");
    }
}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtable_del(struct rtable_t *rtable, char *key){
    if (rtable == NULL || key == NULL){
        return -1;
    }

    struct _MessageT request;

    message_t__init(&request);

    request.opcode = MESSAGE_T__OPCODE__OP_DEL;
    request.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    request.key = key;

    request.data_size = 0;
    request.data = NULL;
    request.keys = NULL;
    request.n_keys = 0;

    if (request.key == NULL){
        return -1;
    }

    struct message_t *req = MTom(&request);
    if (req == NULL || req->key == NULL){
        return -1;
    }

    struct message_t *response = network_send_receive(rtable, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return -1;
    }
    int r = response->data_size;
    free_message_t(response);
    return r;
}

/* Devolve o número de elementos contidos na tabela.
 */
int rtable_size(struct rtable_t *rtable){
    if (rtable == NULL){
        return -1;
    }

    struct _MessageT request;

    message_t__init(&request);

    request.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    request.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    request.data_size = 0;
    request.data = NULL;
    request.key = NULL;
    request.n_keys = 0;
    request.keys = NULL;

    struct message_t *req = MTom(&request);
    if (req == NULL){
        return -1;
    }

    struct message_t *response = network_send_receive(rtable, req);
    if(response == NULL){
        free_message_t(response);
        return -1;
    }

    int nkeys = response->data_size;

    free_message_t(response);
    return nkeys;
}

/* Devolve um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento a NULL.
 */
char **rtable_get_keys(struct rtable_t *rtable){
    if (rtable == NULL){
        return NULL;
    }

    struct _MessageT request;

    message_t__init(&request);

    request.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    request.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    request.data_size = 0;
    request.data = NULL;
    request.key = NULL;
    request.keys = NULL;
    request.n_keys = 0;

    struct message_t *req = MTom(&request);
    if (req == NULL){
        return NULL;
    }

    struct message_t *response = network_send_receive(rtable, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return NULL;
    }

    if (response->n_keys == 0){
        free_message_t(response);
        return NULL;
    }

    char **copyKeys = response->keys;
    free(response);
    return copyKeys;
}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys){
    int i = 0;
    while (keys[i] != NULL){
        free(keys[i]);
        i++;
    }
    free(keys);
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int rtable_verify(struct rtable_t *rtable, int op_n){
    if (rtable == NULL){
        return -1;
    }

    struct _MessageT request;

    message_t__init(&request);

    request.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    request.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    //vou enviar pelo datasize porque quero mandar um int e nao vou precisar dele para outra coisa
    request.data_size = op_n;

    request.data = NULL;
    request.key = NULL;
    request.keys = NULL;
    request.n_keys = 0;

    struct message_t *req = MTom(&request);
    if (req == NULL){
        return -1;
    }printf("OLA\n");

    struct message_t *response = network_send_receive(rtable, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return -1;
    }printf("OLA\n");

    if(response->opcode == MESSAGE_T__OPCODE__OP_VERIFY + 1){
        free_message_t(response);
        return 0;
    } else {
        free_message_t(response);
        return -1;
    }
}
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
#include "zookeeper/zookeeper.h"


/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024

struct rtable_t* head;
struct rtable_t* tail;
static int is_connected;
static char *watcher_ctx = "ZooKeeper Data Watcher";
typedef struct String_vector zoo_string;
zoo_string* children_list;
static zhandle_t *zh;
static char *zoo_path = "/chain";

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            is_connected = 1;
        } else {
            is_connected = 0;
        }
    }
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    int zoo_data_len = ZDATALEN;
    if (state == ZOO_CONNECTED_STATE)	 {
        if (type == ZOO_CHILD_EVENT) {
            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(wzh, zpath, child_watcher, watcher_ctx, children_list)) {
                fprintf(stderr, "Error setting watch at %s!\n", zpath);
            }
    
            ordenaChildren(children_list);

            char lowest_id[120] = "/chain/";
		    strcat(lowest_id,children_list->data[0]); 

            char highest_id[120] = "/chain/";
		    strcat(highest_id,children_list->data[children_list->count-1]); 

            char *address_port_l = (char *) malloc(ZDATALEN * sizeof(char));
            int address_port_l_length = ZDATALEN;
            //o watch vai 0 porque nao mudo o node
            if (ZOK != zoo_get(wzh, lowest_id, 0, address_port_l, &address_port_l_length, NULL)) {
                fprintf(stderr, "Error getting data at %s!\n", zpath);
            }
            head = rtable_connect(address_port_l);
            if (head == NULL){
                rtable_disconnect(head);
                fprintf(stderr, "Error connecting head\n");
            }

            char *address_port_h = (char *) malloc(ZDATALEN * sizeof(char));
            int address_port_h_length = ZDATALEN;
            //o watch vai 0 porque nao mudo o node
            if (ZOK != zoo_get(wzh, highest_id, 0, address_port_h, &address_port_h_length, NULL)) {
                fprintf(stderr, "Error getting data at %s!\n", zpath);
            }
            tail = rtable_connect(address_port_h);
            if (tail == NULL){
                rtable_disconnect(head);
                fprintf(stderr, "Error connecting tail\n");
            }

            free(address_port_h);
            free(address_port_l);
        }
    }
    free(children_list);
}

int zookeeper_connect(char* host_port){
    const char* zoo_root = "/chain";
    children_list =	(zoo_string *) malloc(sizeof(zoo_string*));

    /* Ligar ao zookeeper */
    zh = zookeeper_init(host_port, connection_watcher,	2000, 0, NULL, 0);
    if (zh == NULL)	{
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
        exit(EXIT_FAILURE);
    }
    sleep(3);
    if(is_connected) {
        /* Buscar filhos para children_list e activar watches*/
        if (ZOK != zoo_wget_children(zh, zoo_path, &child_watcher, watcher_ctx, children_list)) {
            fprintf(stderr, "Error setting watch at %s!\n", zoo_path);
        }
    }
    for (int i = 0; i < children_list->count; i++)  {
		fprintf(stderr, "\n(%d): %s\n", i+1, children_list->data[i]);
        printf("data no children: %s\n", children_list->data[i]);
	}

    //obtem servidores head e tail
    ordenaChildren(children_list);

    char lowest_id[120] = "/chain/";
	strcat(lowest_id,children_list->data[0]); 

    char highest_id[120] = "/chain/";
	strcat(highest_id,children_list->data[children_list->count-1]); 

    char *address_port_l = (char *) malloc(ZDATALEN * sizeof(char));
    int address_port_l_length=ZDATALEN;
    //o watch vai 0 porque nao mudo o node
    if (ZOK != zoo_get(zh,lowest_id, 0, address_port_l, &address_port_l_length, NULL)) {
        fprintf(stderr, "Error getting data at %s!\n", zoo_path);
    }

    head = rtable_connect(address_port_l);

    if (head == NULL){
        rtable_disconnect(head);
        fprintf(stderr, "Error connecting head\n");
    }

    char *address_port_h = (char *) malloc(ZDATALEN * sizeof(char));
    int address_port_h_length=ZDATALEN;
    //o watch vai 0 porque nao mudo o node
    if (ZOK != zoo_get(zh,highest_id, 0, address_port_h, &address_port_h_length, NULL)) {
        fprintf(stderr, "Error getting data at %s!\n", zoo_path);
    }
    tail = rtable_connect(address_port_h);
    if (tail == NULL){
        rtable_disconnect(head);
        fprintf(stderr, "Error connecting tail\n");
    }

    free(address_port_h);
    free(address_port_l);

  return zh;
}

int zookeeper_disconect(int zh){
    for (int i = 0; i < children_list->count; i++)  {
        free(children_list->data[i]);
    }
    free(children_list->data);
    free(children_list);
    free(head);
    free(tail);
    free(zh);
    return zookeeper_close(zh);
}

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
    free(rtable);
    return 0;
}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtable_put(struct entry_t *entry){
    if (head == NULL || entry == NULL){
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

    struct message_t *response = network_send_receive(head, req);
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
struct data_t *rtable_get(char *key){
    if (tail == NULL || key == NULL){
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

    struct message_t *response = network_send_receive(tail, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return NULL;
    }
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
int rtable_del(char *key){
    if (head == NULL || key == NULL){
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

    struct message_t *response = network_send_receive(head, req);
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
int rtable_size(){
    if (tail == NULL){
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

    struct message_t *response = network_send_receive(tail, req);
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
char **rtable_get_keys(){
    if (tail == NULL){
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

    struct message_t *response = network_send_receive(tail, req);
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
int rtable_verify(int op_n){
    if (tail == NULL){
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
    }

    struct message_t *response = network_send_receive(tail, req);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        free_message_t(response);
        return -1;
    }

    if(response->opcode == MESSAGE_T__OPCODE__OP_VERIFY + 1){
        free_message_t(response);
        return 0;
    } else {
        free_message_t(response);
        return -1;
    }
}


void ordenaChildren(struct String_vector *children_list){
    char temp[50];

    for (int i = 0; i < children_list->count; ++i) {
        for (int j = i + 1; j < children_list->count; ++j) {
            if (strcmp(children_list->data[i], children_list->data[j]) > 0) {
                strcpy(temp, children_list->data[i]);
                strcpy(children_list->data[i], children_list->data[j]);
                strcpy(children_list->data[j], temp);
            }
        }
    }
}
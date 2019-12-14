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
#include "zookeeper/zookeeper.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

static zhandle_t *zh;
static int is_connected;
static char *root_path = "/chain";
struct rtable_t *servers = NULL;

char hostbuffer[256]; 
char *IPbuffer; 
struct hostent *host_entry; 
int hostname; 
char* adress;
  

#define ZDATALEN 1024 * 1024
typedef struct String_vector zoo_string; 
static char *watcher_ctx = "ZooKeeper Data Watcher";

struct table_t *tabela;

int last_assigned = 0;
int op_count = -1;
struct task_t *queue_head;
pthread_mutex_t queue_lock, table_lock;
pthread_cond_t queue_not_empty;
pthread_t thread;
int run = 1;


/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(char* port,int n_lists, char* ipZoo){

    if(n_lists < 1){
        return -1;
    }
    tabela = table_create(n_lists);
    if(tabela == NULL){
        return -1;
    }

    if(port==NULL){
        return -1;
    }

    if(ipZoo==NULL){
        return -1;
    }

    
    hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
    if (hostname == -1) 
    { 
        perror("gethostname"); 
        exit(1); 
    } 
  
    
    host_entry = gethostbyname(hostbuffer); 
    if (host_entry == NULL) 
    { 
        perror("gethostbyname"); 
        exit(1); 
    } 
  
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); 
  
    adress = strcat(IPbuffer, ":");
    strcat(adress,port);

    

    /* criação de nova thread */
	if (pthread_create(&thread, NULL, &process_task, NULL) != 0){
		perror("\nThread não criada.\n");
		exit(EXIT_FAILURE);
	}
    pthread_mutex_init(&queue_lock, NULL);
    pthread_mutex_init(&table_lock, NULL);
    pthread_cond_init(&queue_not_empty, NULL);





    servers = (struct rtable_t*) malloc(sizeof(struct rtable_t));
   
    
    servers->zh = zookeeper_init(ipZoo, connection_watcher,	2000, 0, NULL, 0); 
	if (servers->zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}
    sleep(3);
    if (is_connected) {
       
		if (ZNONODE == zoo_exists(servers->zh, root_path, 0, NULL)) {
			if (ZOK != zoo_create(servers->zh, root_path, NULL, -1, & ZOO_CREATOR_ALL_ACL, 0, NULL, 0)) {
			    fprintf(stderr, "Error creating znode from path %s!\n", root_path);
			    exit(EXIT_FAILURE);
		    }
		}
    
		char node_path[120] = "";
        
		strcat(node_path,root_path); 
		strcat(node_path,"/node"); 
		int new_path_len = 1024;
		char* new_path = malloc (new_path_len);
        
        
		
		if (ZOK != zoo_create(servers->zh, node_path, adress, strlen(adress), & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len)) {
			fprintf(stderr, "Error creating znode from path %s!\n", node_path);
			exit(EXIT_FAILURE);
		}
        
		fprintf(stderr, "Ephemeral Sequencial ZNode created! ZNode path: %s\n", new_path); 
        if (sscanf(new_path, "%d", &servers->idZoo) != 1 && new_path_len==0) {
            printf("Erro ao obter id de node\n");
        }
		free (new_path);
        

        zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
        if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", root_path);
	    }
	    fprintf(stderr, "\n=== znode listing === [ %s ]", root_path); 
        int seqNumber=-1;
        int indice=-1;
	    for (int i = 0; i < children_list->count; i++)  {
		    fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]);
            printf("data no children: %s\n", children_list->data[i]);
            if (sscanf(children_list->data[i], "%d", &seqNumber) != 1) {
                printf("Erro ao obter id de node\n");
            }
            if(seqNumber ==servers->idZoo){
                indice=i;
            }
	    }
        if(indice!=-1 && indice!=children_list->count-1){
            if(children_list->data[indice+1]!=NULL){
                int next=-1;
                if (sscanf(children_list->data[indice+1], "%d", &next) != 1) {
                    printf("Erro ao obter id de node\n");
                }
                if(next!=-1){
                    servers->idNext=children_list->data[indice+1];
                    servers->sockfd;
                }
            }   
        } 
        else if(indice!=-1 && indice==children_list->count-1){
            servers->idNext=-1;
            servers->sockfd=-1;
        }

        fprintf(stderr, "\n=== done ===\n");

        //fazWatch();
	}

    ///////
    
    return 0;
}

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy(){
    if(tabela != NULL){
        table_destroy(tabela);
    }
    run=0;
    pthread_cond_signal(&queue_not_empty);
    if (pthread_join(thread,NULL) != 0){
		perror("\nErro no join.\n");
		exit(EXIT_FAILURE);
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

            result=insereTask(0,msg->key,NULL);

            if(result != -1 ){
                msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
                msg->data_size = result;
            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }
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
            result=insereTask(1,msg->key,msg->data);
            if(result != -1 ){
                msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
                msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
                msg->data_size = result;

            } else{
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }
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
            printf("A espera de tasks ...\n");
            pthread_cond_wait(&queue_not_empty,&queue_lock);
            if(!run){
                return;
            }
        }
        
        if(queue_head!=NULL){

            struct task_t *atual = queue_head;
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
                        pthread_mutex_lock(&table_lock);
                        struct data_t *d=data_create2(strlen(atual->data)+1, atual->data);
                        table_put(tabela, atual->key, d);
                        pthread_mutex_unlock(&table_lock);
                        data_destroy(d);
                        op_count++;
                    }
                }
                free(atual->key);
                struct task_t *a = atual;
                atual=atual->next;
                free(a);
            }
            queue_head=atual;
        }
        pthread_mutex_unlock(&queue_lock);
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
 // fase 4

 /**
* Watcher function for connection state change events
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
    printf("path=%s\n", path);
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
            printf("==================if====================\n");

		} else {
			is_connected = 0; 
            printf("================else======================\n");

		}
	}
}
/*
void fase4(char* ipZoo){
    
    servers->zh = zookeeper_init(ipZoo, connection_watcher,	2000, 0, NULL, 0); 
	if (servers->zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}
    sleep(3);
    if (is_connected) {

		if (ZNONODE == zoo_exists(servers->zh, "", 0, NULL)) {
			fprintf(stderr, "%s doesn't exist! \
				            Try again \n", root_path);
			exit(EXIT_FAILURE);
		}

		char node_path[120] = "";
        
		//strcat(node_path,root_path); 
		strcat(node_path,"/node"); 
		int new_path_len = 1024;
		char* new_path = malloc (new_path_len);
        adress="127.0.0.1:10001";
        
		
		if (ZOK != zoo_create(zh, node_path, adress, sizeof(adress), & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len)) {
			fprintf(stderr, "Error creating znode from path %s!\n", node_path);
			exit(EXIT_FAILURE);
		}
        
		fprintf(stderr, "Ephemeral Sequencial ZNode created! ZNode path: %s\n", new_path); 
        if (sscanf(new_path, "%d", &servers->idZoo) != 1 && new_path_len==0) {
            printf("Erro ao obter id de node\n");
        }
		free (new_path);
        fazWatch();
	}

}*/


/**
* Data Watcher function for /MyData node
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	int zoo_data_len = ZDATALEN;
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
	 	   /* Get the updated children and reset the watch */ 
 			if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", root_path);
 			}
			fprintf(stderr, "\n=== znode listing === [ %s ]", root_path); 
			for (int i = 0; i < children_list->count; i++)  {
				fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]);
			}
			fprintf(stderr, "\n=== done ===\n");
		 } 
	 }
	 free(children_list);
}

void fazWatch(){
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", root_path);
	}
	fprintf(stderr, "\n=== znode listing === [ %s ]", root_path); 
    int seqNumber=-1;
    int indice=-1;
	for (int i = 0; i < children_list->count; i++)  {
		fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]);
        printf("data no children: %s\n", children_list->data[i]);
        if (sscanf(children_list->data[i], "%d", &seqNumber) != 1) {
            printf("Erro ao obter id de node\n");
        }
        if(seqNumber ==servers->idZoo){
            indice=i;
        }
	}
    if(indice!=-1 && indice!=children_list->count-1){
        if(children_list->data[indice+1]!=NULL){
            int next=-1;
            if (sscanf(children_list->data[indice+1], "%d", &next) != 1) {
                printf("Erro ao obter id de node\n");
            }
            if(next!=-1){
                servers->idNext=children_list->data[indice+1];
                servers->sockfd;
            }
        }
    } 
    else if(indice!=-1 && indice==children_list->count-1){
        servers->idNext=-1;
        servers->sockfd=-1;
    }

    fprintf(stderr, "\n=== done ===\n");

}
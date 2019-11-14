/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */


#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "table_skel.h"
#include "network_server.h"
#include "message.h"
#include "table.h"
#include "inet.h"
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#define NFDESC 10 // Numero de sockets (uma para listening)
#define TIMEOUT 50 // em milisegundos

int sockfd, connsockfd;
struct sockaddr_in server, client;
int nbytes, opt, kfds, nfds;
socklen_t size_client;
struct pollfd connections[NFDESC];

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port){

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(setsockopt(sockfd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*) &opt,sizeof(opt)) < 0){
        printf("setsockopt failed\n");
        close(sockfd);
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind\n");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen\n");
        close(sockfd);
        return -1;
    }
    

    return sockfd;
}


/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket){
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    printf("Servidor à espera de dados.. %d\n",sockfd);

    for (int i = 0; i < NFDESC; i++){
        connections[i].fd = -1;    // poll ignora estruturas com fd < 0
    }

    connections[0].fd = sockfd;  // Vamos detetar eventos na welcoming socket
    connections[0].events = POLLIN; 

    nfds = 1; // numero de file descriptors

    while(1){
        while ((kfds = poll(connections, nfds, TIMEOUT)) >= 0){
        if (kfds > 0){ // kfds eh o numero de descritores com evento ou erro

            if ((connections[0].revents & POLLIN) && (nfds < NFDESC)){  // Pedido na listening socket ?
                if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Ligacao feita ?
                    connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
                    nfds++;
                }
            }
            for (int i = 1; i < nfds; i++){// Todas as ligacoes
                if (connections[i].revents & POLLIN) { // Dados para ler ?
                    // get request message from client
                    struct message_t *msg = network_receive(connections[i].fd);

                    if(msg==NULL){
                        close(connections[i].fd);
                        //remover elemento
                        nfds--;
                    }
                    else{
                        if(invoke(msg) != 0){
                            free_message_t(msg);
                            perror("Erro ao construir mensagem de resposta");
                            close(connections[i].fd);
                            return -1;
                        }

                        if(network_send(connections[i].fd, msg) < 0){
                            free_message_t(msg);
                            perror("Erro ao enviar dados ao cliente");
                            close(connections[i].fd);
                            return -1;
                        }
                    }
                    free_message_t(msg);
                }
                if(connections[i].revents & (POLLERR | POLLHUP)){
                     close(connections[i].fd);
                     //remover elemento
                }
            }
        }
    }       
}
    return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket){
    if(client_socket < 0){
        return NULL;
    }

    int len;

    if((nbytes = read_all(client_socket,&len, sizeof(len))) < 0){
        perror("Erro ao receber dados do cliente");
        close(client_socket);
    }

    char *str = (char*) malloc(len);

    if((nbytes = read_all(client_socket,str,len)) < 0){
        perror("Erro ao receber dados do cliente");
        close(client_socket);
    }

    MessageT *m ;

    m = message_t__unpack(NULL, nbytes,str);
    if(m == NULL){
        return NULL;
    }
    struct message_t *msg = MTom(m);
    message_t__free_unpacked(m ,NULL);
    free(str);
    return msg;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t *msg){
    MessageT m = mToM(msg);
    int len = message_t__get_packed_size(&m);
    char *buf = malloc(len);
    message_t__pack(&m, buf);
    if(buf == NULL){
        return -1;
    }

    if(write_all(client_socket,&len, sizeof(len)) != sizeof(len)){
        perror("Erro ao enviar dados do cliente");
        close(client_socket);
        return -1;
    }

    if(write_all(client_socket,buf,len) != len){
        perror("Erro ao enviar dados do cliente");
        close(client_socket);
        return -1;
    }
    free(buf);
    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    table_skel_destroy();
    return close(sockfd);
}


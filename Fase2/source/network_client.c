/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include "sdmessage.pb-c.h"
#include "message.h"
#include "client_stub-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <unistd.h>


/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable){
    // Estabelece conexão com o servidor definido em server
    struct sockaddr_in server = rtable->address;
    if (connect(rtable->sockfd, (const struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(rtable->sockfd);
        return -1;
    }
    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtable_t *rtable,
                                       struct message_t *msg){

    int sockfd = rtable->sockfd;
    uint8_t *buf = NULL;
    int len;

    MessageT message = mToM(msg);

    len = message_t__get_packed_size(&message);

    buf = malloc(len);
    if (buf == NULL){
        free_message_t(msg);
        fprintf(stdout, "malloc error\n");
        return NULL;
    }

    message_t__pack(&message, buf);

    int nbytes;

    if((nbytes = write_all(sockfd, &len, sizeof(len))) != sizeof(len)){
        free_message_t(msg);
        free(buf);
        perror("Erro ao enviar dados ao servidor");
        close(sockfd);
        return NULL;
    }

    if((nbytes = write_all(sockfd, buf, len)) != len){
        free_message_t(msg);
        free(buf);
        perror("Erro ao enviar dados ao servidor");
        close(sockfd);
        return NULL;
    }

    printf("À espera de resposta do servidor ...\n");

    int recv_len;

    if((nbytes = read_all(sockfd, &recv_len, sizeof(recv_len))) != sizeof(recv_len)){
        free_message_t(msg);
        free(buf);
        perror("Erro ao receber dados do servidor");
        close(sockfd);
        return NULL;
    }

    char *recv_buf = (char *) malloc(recv_len);

    if((nbytes = read_all(sockfd, recv_buf, recv_len)) != recv_len){
        free_message_t(msg);
        free(buf);
        free(recv_buf);
        perror("Erro ao receber dados do servidor");
        close(sockfd);
        return NULL;
    };

    MessageT *recv_msg;

    recv_msg = message_t__unpack(NULL, recv_len, recv_buf);
    if (recv_msg == NULL) {
        free_message_t(msg);
        free(buf);
        free(recv_buf);
        free(recv_msg);
        fprintf(stdout, "error unpacking message\n");
        return NULL;
    }

    free_message_t(msg);
    free(buf);
    free(recv_buf);
    struct message_t *msg2 = MTom(recv_msg);
    message_t__free_unpacked(recv_msg, NULL);
    return msg2;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtable_t * rtable){
    if (rtable == NULL){
        return -1;
    }
    return close(rtable->sockfd);
}

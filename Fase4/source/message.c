/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include "sdmessage.pb-c.h"
#include "message.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int write_all(int sock, char *buf, int len) {
    if (buf == NULL){
        return -1;
    }
    int bufsize = len;
    while(len > 0) {
        int res = write(sock, buf, len);
        if(res < 0) {
            if(errno==EINTR) continue;
            perror("write failed:");
            return res;
        }
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, char *buf, int len) {
    int timeout = 0;
    if (buf == NULL){
        return -1;
    }
    int bufsize = len;
    while(len > 0) {
        int res = read(sock, buf, len);
        if(res < 0) {
            if(errno==EINTR) continue;
            perror("read failed:");
            return res;
        }
        if (res == 0){
            timeout++;
        }
        //isto podia ser menor mas ou bem
        if (timeout == 10){
            if(errno==EINTR) continue;
            perror("read failed:");
            return res;
        }
        buf += res;
        len -= res;
    }
    return bufsize;
}

struct message_t* MTom(MessageT *msg){
    if(msg == NULL){
        return NULL;
    }
    struct message_t *mensagem = (struct message_t*) malloc(sizeof(struct message_t));

    if (mensagem == NULL){
        return NULL;
    }

    mensagem->base = msg->base;
    mensagem->opcode = msg->opcode;
    mensagem->c_type = msg->c_type;
    mensagem->data_size = msg->data_size;
    mensagem->data = NULL;
    mensagem->key = NULL;
    mensagem->n_keys = msg->n_keys;
    mensagem->keys = NULL;

    if (msg->data != NULL && strcmp(msg->data, "") != 0) {
        mensagem->data = strdup(msg->data);
    }

    if (msg->key != NULL && strcmp(msg->key, "") != 0) {
        mensagem->key = strdup(msg->key);
    }

    int i = 0;
    if (mensagem->n_keys > 0){
        i = mensagem->n_keys;
    }
    if (msg->keys != NULL) {
        mensagem->keys = (char**) malloc((i+1) * sizeof(char*));
        if (mensagem->keys == NULL){
            return NULL;
        }
        for(int j = 0; j < i; j++)
        {
            mensagem->keys[j] = strdup(msg->keys[j]);
        }
        mensagem->keys[i] = NULL;
    }
    return mensagem;
}

struct _MessageT mToM(struct message_t *msg){
    MessageT mensagem;
    message_t__init(&mensagem);

    mensagem.base = msg->base;
    mensagem.opcode = msg->opcode;
    mensagem.c_type = msg->c_type;
    mensagem.data_size = msg->data_size;
    mensagem.data = msg->data;
    mensagem.key = msg->key;
    mensagem.n_keys = msg->n_keys;
    mensagem.keys = msg->keys;
    return mensagem;
}

void free_message_t(struct message_t *msg){
    if (msg == NULL){
        return;
    }
    if (msg->keys != NULL) {
        for (int i = 0; i < msg->n_keys; i++) {
            free(msg->keys[i]);
        }
        free(msg->keys);
    }
    if (msg->key != NULL) {
        free(msg->key);
    }
    if (msg->data != NULL) {
        free(msg->data);
    }
    free(msg);
}
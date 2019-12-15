/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include <netinet/in.h>
#include "client_stub.h"
#include "zookeeper/zookeeper.h"


struct rtable_t{
    struct sockaddr_in address;
    int sockfd;
};

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

int zookeeper_connect(char* host_port);

int zookeeper_disconect(int zh);

void ordenaChildren(struct String_vector *children_list);

#endif 

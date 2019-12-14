/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "sdmessage.pb-c.h"
#include "table.h"
#include <netinet/in.h>
#include "zookeeper/zookeeper.h"
#include "network_server.h"



/* Insere uma task na struct task_t
 * Devolve o numero da operação
*/
int insereTask(int op,char* key, char *data);

struct server_t{
    zhandle_t *zh;
    char* id;
    char* idNext;
    int sockfd;
};

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

void compareFunction(struct String_vector *children_list);

#endif

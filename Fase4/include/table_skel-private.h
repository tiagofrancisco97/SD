/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "sdmessage.pb-c.h"
#include "table.h"

/* Insere uma task na struct task_t
 * Devolve o numero da operação
*/
int insereTask(int op,char* key, char *data);

#endif

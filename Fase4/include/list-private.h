/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "list.h"

struct node_t {
	struct entry_t *entry;
	struct node_t *next;
};

struct list_t {
	struct node_t *first;
    	struct node_t *last;
    	int size;
};

void list_print(struct list_t* list);

#endif

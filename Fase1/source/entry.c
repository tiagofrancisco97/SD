/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include "entry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados passados.
 */
struct entry_t *entry_create(char *key, struct data_t *data){

    if(key==NULL || data == NULL ){
        return NULL;
    }

    struct entry_t *entry = malloc (sizeof (*entry));

    entry->key = key;
    entry->value = data;
    return entry;
}

/* Função que inicializa os elementos de uma entrada na tabela com o
 * valor NULL.
 */
void entry_initialize(struct entry_t *entry){

    entry->key=NULL;
    entry->value=NULL;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry){
    if(entry!=NULL){
        data_destroy(entry->value);
        free(entry->key);
        free(entry);
    }
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry){
    
    if(entry==NULL || entry->key == NULL || entry->value==NULL){
        return NULL;
    }

    struct entry_t *new = malloc (sizeof (*new));

    new->value=data_dup(entry->value);

    new->key=strdup(entry->key);

    return new;
}
/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */


#ifndef _ENTRY_H
#define _ENTRY_H /* Módulo entry */

#include "data.h"

/* Esta estrutura define o par {chave, valor} para a tabela
 */
struct entry_t {
	char *key;	/* string, cadeia de caracteres terminada por '\0' */
	struct data_t *value; /* Bloco de dados */
};

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados passados.
 */
struct entry_t *entry_create(char *key, struct data_t *data);

/* Função que inicializa os elementos de uma entrada na tabela com o
 * valor NULL.
 */
void entry_initialize(struct entry_t *entry);

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry);

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry);

#endif


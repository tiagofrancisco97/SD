/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "list.h"

struct table_t {
   int lines;
   struct list_t **lists;
};

/* Função para criar/inicializar uma nova tabela hash, com n linhas 
 * (módulo da função HASH).
 */
struct table_t *table_create(int n);

/*
 * Função que recebe uma chave e o numero de linhas da tabela
 * Soma o ASCII da chave e retorna o resto da soma pelo numero de linhas
 * É a nossa funcao hash para encontrar a linha a que cada chave pertence
 */
int get_index(char *key, int n);

void table_print(struct table_t *table);

#endif
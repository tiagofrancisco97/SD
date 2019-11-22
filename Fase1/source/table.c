/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */


#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "table-private.h"
#include "list.h"

/* Função para criar/inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash)
 * Em caso de erro retorna NULL.
 */
struct table_t *table_create(int n){
	if (n < 1){
		return NULL;
	}

	struct table_t *table = (struct table_t*) malloc(sizeof(*table));
	if (table == NULL){
		return NULL;
	}

	table->lines = n;

	//table->lists = calloc(n, sizeof(struct list_t*));
	table->lists = (struct list_t**) malloc(n * sizeof(struct list_t*));
	if (table->lists == NULL){
		return NULL;
	}

	for (int i = 0; i < n; i++){
		table->lists[i] = list_create();
	}

	//memcpy(table->lists, array, n * sizeof(struct list_t));
	return table;
}

/* Função para libertar toda a memória ocupada por uma tabela.
 */
void table_destroy(struct table_t *table){
	if (table == NULL || table->lists == NULL){
		return;
	}
	for (int i = 0; i < table->lines; i++){
		list_destroy(table->lists[i]);
	}
	//NOTA: capaz de nao precisar deste
	free(table->lists);
	free(table);
}

/*
 * Função que recebe uma chave e o numero de linhas da tabela
 * Soma o ASCII da chave e retorna o resto da soma pelo numero de linhas
 * É a nossa função hash para encontrar a linha a que cada chave pertence
 */
int get_index(char *key, int n){
	int i, c = 0;
	for (i = 0; key[i] != '\0'; i++ ){
		c = c + key[i];
	}
	//se a string for vazia
	if (c == 0){
		return -1;
	}
	return c % n;
}

/* Função para adicionar um par chave-valor à tabela.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na tabela,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
//NOTA uma key acaba sempre na mesma linha/lista, logo implementar
//o caso em que a key eh igual sera melhor na lista
int table_put(struct table_t *table, char *key, struct data_t *value){
	if (table == NULL || key == NULL || value == NULL){
		return -1;
	}
	int index = get_index(key, table->lines);
	if (index < 0 || index >= table->lines){
		return -1;
	}
	char *newKey = malloc(strlen(key) + 1);
	memcpy(newKey, key, strlen(key) + 1);
	struct data_t *newValue = data_dup(value);
	struct entry_t *item = entry_create(newKey, newValue);
	//printf("Vou mandar inserir entry de key %s e data %s na lista de indice %d\n", newKey, newValue->data, index);
	return list_add(table->lists[index], item);
}

/* Função para obter da tabela o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou table_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da tabela,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função.
 * Devolve NULL em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key){
	if (table == NULL || key == NULL){
		return NULL;
	}
	int index = get_index(key, table->lines);
	if (index < 0 || index >= table->lines){
		return NULL;
	}
	struct entry_t *item = list_get(table->lists[index], key);
	if (item == NULL){
		return NULL;
	} else {
		return data_dup(item->value);
	}

}

/* Função para remover um elemento da tabela, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação table_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int table_del(struct table_t *table, char *key){
	struct data_t *teste = table_get(table, key);
	if (table == NULL || key == NULL || teste == NULL){
		return -1;
	}
	data_destroy(teste);
	int index = get_index(key, table->lines);
	if (index < 0 || index >= table->lines){
		return -1;
	}
	return list_remove(table->lists[index], key);
}

/* Função que devolve o número de elementos contidos na tabela.
 */
int table_size(struct table_t *table){
	int i, c = 0;
	for (i = 0; i < table->lines; i++){
		c = c + list_size(table->lists[i]);
	}
	return c;
}


/* Função que devolve um array de char* com a cópia de todas as keys da
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **table_get_keys(struct table_t *table){
	if (table == NULL){
		return NULL;
	}
    char **keys = malloc((table_size(table) + 1) * sizeof(char*));
	int i, j, index = 0;
	for (i = 0; i < table->lines; i++){
		char** list_keys = list_get_keys(table->lists[i]);
		for(j = 0; j < list_size(table->lists[i]); j++){
			int length = strlen(list_keys[j]) + 1;
			keys[index] = (char*) malloc(length);
			if (keys[index] == NULL){
				return NULL;
			}
            memcpy(keys[index], list_keys[j], length);
            index++;
		}
		list_free_keys(list_keys);
	}
	keys[index] = NULL;
	return keys;
}

/* Função que liberta toda a memória alocada por table_get_keys().
 */
void table_free_keys(char **keys){
	int i = 0;
	    while (keys[i] != NULL){
	        free(keys[i]);
	        i++;
	    }
	    free(keys);
}













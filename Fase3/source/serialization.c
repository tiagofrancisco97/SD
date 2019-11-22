/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "data.h"
#include "entry.h"

/* Serializa uma estrutura data num buffer que será alocado
 * dentro da função. Além disso, retorna o tamanho do buffer
 * alocado ou -1 em caso de erro.
 */
int data_to_buffer(struct data_t *data, char **data_buf){
	if (data == NULL){
		return -1;
	} else if (data_buf == NULL){
		return -1;
	}

	*data_buf = malloc(sizeof(data->datasize) + data->datasize);
	if (*data_buf == NULL){
		return -1;
	}
	memcpy(*data_buf, (char*) &data->datasize, sizeof(data->datasize));
	memcpy(*data_buf + sizeof(data->datasize), data->data, data->datasize);
	//Fica escrito datasize/data
	return data->datasize + sizeof(data->datasize);
}

/* De-serializa a mensagem contida em data_buf, com tamanho
 * data_buf_size, colocando-a e retornando-a numa struct
 * data_t, cujo espaco em memoria deve ser reservado.
 * Devolve NULL em caso de erro.
 */
struct data_t *buffer_to_data(char *data_buf, int data_buf_size){
	if (data_buf_size < 1){
		return NULL;
	} else if (data_buf == NULL){
		return NULL;
	}

	int datasize = *(int*)(data_buf);
	char *data = malloc(datasize);
	if (data == NULL){
		return NULL;
	}

	memcpy(data, data_buf + sizeof(int), datasize);
	struct data_t *item = data_create2(datasize, (void*) data);
	return item;
}

/* Serializa uma estrutura entry num buffer que sera alocado
 * dentro da função. Além disso, retorna o tamanho deste
 * buffer alocado ou -1 em caso de erro.
 */

int entry_to_buffer(struct entry_t *data, char **entry_buf){
	if (data == NULL){
		return -1;
	} else if (entry_buf == NULL){
		return -1;
	}
    int data_size = data_to_buffer(data->value, entry_buf);
    if (data_size < 1){
    	return -1;
    }

    int length = strlen(data->key) + 1;
    *entry_buf = (char*) realloc(*entry_buf, data_size + length + sizeof(int));
    if (*entry_buf == NULL){
    	return -1;
    }
    memcpy(*entry_buf + data_size, (char*) &length, sizeof(int));
    memcpy(*entry_buf + data_size + sizeof(int), data->key, length);

	//fica escrito datasize/data/length/key
    return data_size + length + sizeof(int);
}


/* De-serializa a mensagem contida em entry_buf, com tamanho
 * entry_buf_size, colocando-a e retornando-a numa struct
 * entry_t, cujo espaco em memoria deve ser reservado.
 * Devolve NULL em caso de erro.
 */

struct entry_t *buffer_to_entry(char *entry_buf, int entry_buf_size){
	if (entry_buf == NULL){
		return NULL;
	} else if (entry_buf_size < 1){
		return NULL;
	}

	struct data_t *value = buffer_to_data(entry_buf, entry_buf_size);
    int offset = value->datasize + sizeof(int);
    int length = *(int*)(entry_buf + offset);
    char* key = malloc(length);
    if (key == NULL){
    	return NULL;
    }
    memcpy(key, entry_buf + offset + sizeof(int), length);
    return entry_create(key, value);
}



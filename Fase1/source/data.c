/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include "data.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size){
	if (size <= 0) { 
    	return NULL;	
  	}

	struct data_t *val = malloc (sizeof (*val));
    if (val == NULL){
		return NULL;
	}

	val->data = malloc (size);
    if (val->data == NULL) {
        free (val);
        return NULL;
    }

	val->datasize = size;
	return val;
}

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void *data){
	if (size <= 0 || data == NULL) { 
    	return NULL;	
  	}

	struct data_t *val = malloc (sizeof (*val));
    if (val == NULL){
		return NULL;
	}

	val->data = data;
	val->datasize = size;
	
	return val;
} 

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data){

	if(data!=NULL){
		free(data->data);
		free(data); 
	}
}

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 */
struct data_t *data_dup(struct data_t *data){

	if(data == NULL || data->data == NULL || data->datasize <= 0){
		return NULL;
	}
	
	struct data_t *newVal = malloc (sizeof (*newVal));

	 if(newVal == NULL){
		 return NULL;
	 }
        
	newVal->data = malloc (data->datasize);

	 if (newVal->data == NULL) {
        free (newVal);
        return NULL;
    }

	memcpy(newVal->data, data->data, data->datasize);
	newVal->datasize = data->datasize;
	return newVal;
}

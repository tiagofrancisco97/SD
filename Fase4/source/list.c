/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "list-private.h"
#include "data.h"
#include "entry.h"


/* Função que cria uma nova lista (estrutura list_t a ser definida pelo
 * grupo no ficheiro list-private.h).
 * Em caso de erro, retorna NULL.
 */
struct list_t *list_create(){
    struct list_t *list =(struct list_t*) malloc(sizeof(*list));
    list->first = NULL;
    list->last=NULL;
    list->size=0;
    return list;
}

/* Função que elimina uma lista, libertando *toda* a memoria utilizada
 * pela lista.
 */
void list_destroy(struct list_t *list){
    struct node_t *current=list->first;
    while(current!=NULL){
        entry_destroy(current->entry);
        struct node_t *old= current;
        current=current->next;
        free(old);

    }
    free(list);
}

/* Função que adiciona no final da lista (tail) a entry passada como
 * argumento.
 * Retorna 0 (OK) ou -1 (erro).
 */
int list_add(struct list_t *list, struct entry_t *entry){
    if(entry==NULL || list==NULL){
        return -1;
    }

    if(list_get(list,entry->key)!=NULL){
        struct node_t *current=list->first;
        while(current!=NULL){
            if(strcmp(current->entry->key,entry->key)==0){
                entry_destroy(current->entry);
                current->entry=entry;
                return 0;
            }
            current=current->next;
        }
        return -1;
    }

    struct node_t *newE=(struct node_t *) malloc(sizeof(*newE));
    newE->entry=entry;
    newE->next=NULL;

    if(list->first==NULL){
        list->first=newE;
        list->last=newE;
        list->size++;
        return 0;
    }

    if(list->last ==list->first){
        list->first->next = newE;
        list->last=newE;
        list->size++;
        return 0;
    }

    list->last->next=newE;
    list->last=newE;
    list->size++;
    return 0;
}

/* Função que elimina da lista a entry com a chave key.
 * Retorna 0 (OK) ou -1 (erro).
 */
int list_remove(struct list_t *list, char *key){
    if(key==NULL || list==NULL){
        return -1;
    }

    struct node_t *current=list->first;
    if(strcmp(current->entry->key,key)==0){
        list->first=current->next;
        entry_destroy(current->entry);
        free(current);
        list->size--;
        return 0;
    }
    while(current!=NULL){
        if(strcmp(current->next->entry->key,key)==0){
            //Se a key a removida foi a que estava no last, alteramos
            //o apontador do last para o novo ultimo elemento da lista(anterior=penultimo)
            if(strcmp(key,list->last->entry->key)==0){
                list->last=current;
            }
            struct node_t *new=current->next;
            entry_destroy(new->entry);
            current->next=current->next->next;
            list->size--;
            free(new);
            return 0;
        }
        current=current->next;
    }
    return -1;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL em caso de erro.
 * Obs: as funções list_remove e list_destroy vão libertar a memória
 * ocupada pela entry ou lista, significando que é retornado NULL
 * quando é pretendido o acesso a uma entry inexistente.
*/
struct entry_t *list_get(struct list_t *list, char *key){
    if(list==NULL || key==NULL ){
        return NULL;
    }

    struct node_t *atual = list->first;
    while(atual!=NULL){
        if(strcmp(atual->entry->key, key)==0){
            return atual->entry;
        }
        if(atual->next==NULL){
            return NULL;
        }
        atual=atual->next;
    }
	return NULL;
}

/* Função que retorna o tamanho (número de elementos (entries)) da lista,
 * ou -1 (erro).
 */
int list_size(struct list_t *list){
    return list->size;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **list_get_keys(struct list_t *list){
    if(list==NULL){
        return NULL;
    }
    char **keys=malloc((list->size+1)*sizeof(char*));
    struct node_t *current= list->first;
    for (int i = 0; i < list->size || current!=NULL; i++){
        keys[i]= strdup(current->entry->key);
        current=current->next;
    }
    keys[list->size]=NULL;
    free(current);

    return keys;

}

/* Função que liberta a memória ocupada pelo array das keys da tabela,
 * obtido pela função list_get_keys.
 */
void list_free_keys(char **keys){
    int i=0;
    while (keys[i]!=NULL){
        free(keys[i]);
        i++;
    }

    free(keys);
}

void list_print(struct list_t* list){
    struct node_t *current= list->first;
    while(current!=NULL){
        printf("Elemento da lista: key %s e value %p\n", current->entry->key,current->entry->value->data);
        if(current->next==NULL){
            break;
        }
        current=current->next;
    }
}

/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "sdmessage.pb-c.h"
#include "table.h"

struct task_t {
    int op_n; //o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    char* data; // os dados a adicionar em caso de put, ou NULL em caso de delete
//adicionar campo(s) necessário(s) para implementar fila do tipo produtor/consumidor
    struct task_t* next;	
};

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists);

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy();

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
//int invoke(struct message_t* msg);

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n);

/* Função do thread secundário que vai processar pedidos de escrita.
*/
void * process_task (void *params);
#endif

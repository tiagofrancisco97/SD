/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "network_server.h"
#include "table_skel.h"

int main(int argc, char **argv){

    if(argc != 4){
        printf("Deve introduzir: table-server <port> <n_lists> <IP>:<porta>\n");
        printf("Exemplo de uso: ./binary/table-server 12345 2 127.0.0.1:2181 \n");
        return -1;
    }

    int socket_de_escuta = network_server_init(atoi(argv[1]));

    if(socket_de_escuta == -1){
        return -1;
    }

    obtemIp(argv[3]);
    obtemPort(argv[1]);
	int skel_init = table_skel_init(atoi(argv[2]));

    if((skel_init == 0 ) != 0){
        printf("Tabela iniciada\n");
    }else{
        printf("Tabela nao iniciada\n");
        return -1;
    }

    printf("O socket_de_escuta eh %d\n", socket_de_escuta);
    int result = network_main_loop(socket_de_escuta);

    network_server_close(socket_de_escuta);

    return result;
}

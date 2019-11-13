/* Grupo 05
 Carlos Caritas nº51728
 Carlos Marques nº51964
 Tiago Gonçalves nº51729 */

#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "sdmessage.pb-c.h"


struct message_t {
  ProtobufCMessage base;
  MessageT__Opcode opcode;
  MessageT__CType c_type;
  int32_t data_size;
  char *data;
  char *key;
  size_t n_keys;
  char **keys;
};

int write_all(int sock, char *buf, int len);

int read_all(int sock, char *buf, int len);

struct message_t* MTom(MessageT *msg);

struct _MessageT mToM(struct message_t *msg);

void free_message_t(struct message_t *msg);

#endif 

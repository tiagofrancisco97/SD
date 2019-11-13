CC = gcc
CFLAGS = $(CC) -g -Wall -I $(INC) -c 
PROTOFLAG = -I/usr/local/include -L/usr/local/lib -lprotobuf-c

BIN = binary/
INC = include/
OBJ = object/
SRC = source/
LIB = lib/
PROJETO1 = $(OBJ)data.o $(OBJ)list.o $(OBJ)table.o $(OBJ)entry.o
CLIENT_LIB = $(OBJ)client_stub.o $(OBJ)network_client.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)message.o
TABLE_CLIENT = $(OBJ)table-client.o $(OBJ)sdmessage.pb-c.o
TABLE_SERVER = $(OBJ)message.o $(OBJ)table_skel.o $(OBJ)network_server.o $(OBJ)table-server.o $(OBJ)sdmessage.pb-c.o $(PROJETO1)

all : table.o client_stub.o network_client.o message.o table-client.o table_skel.o network_server.o table-server.o sdmessage.pb-c.o \
	  data.o entry.o table.o list.o \
	  client-lib.o table-client table-server


%.o: $(SRC)%.c $($@)
	$(CFLAGS) $< -o $(OBJ)$@

client-lib.o: $(CLIENT_LIB)
	ld -r $(CLIENT_LIB) -o $(LIB)cliente-lib.o

table-client: $(TABLE_CLIENT)
	$(CC) $(LIB)cliente-lib.o $(TABLE_CLIENT) $(PROTOFLAG) -o $(BIN)table-client

table-server: $(TABLE_SERVER)
	$(CC) $(TABLE_SERVER) $(PROTOFLAG) -o $(BIN)table-server

clean:
	rm -f $(BIN)*
	rm -f $(OBJ)*
	rm -f $(LIB)*

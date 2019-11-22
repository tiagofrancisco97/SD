CC = gcc
BIN = binary/
INC = include/
OBJ = object/
SRC = source/
CFLAGS = $(CC) -g -Wall -I $(INC) -c
TESTE_DATA = $(OBJ)test_data.o $(OBJ)data.o
TESTE_ENTRY = $(OBJ)test_entry.o $(OBJ)entry.o $(OBJ)data.o
TESTE_LIST = $(OBJ)test_list.o $(OBJ)entry.o $(OBJ)list.o $(OBJ)data.o
TESTE_TABLE = $(OBJ)test_table.o $(OBJ)data.o $(OBJ)list.o $(OBJ)table.o $(OBJ)entry.o
TESTE_SERIALIZATION = $(OBJ)test_serialization.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)serialization.o

all : data.o test_data.o test_entry.o entry.o list.o test_list.o serialization.o test_serialization.o table.o test_table.o\
	  test_data test_entry test_list test_table test_serialization 


%.o: $(SRC)%.c $($@)
	$(CFLAGS) $< -o $(OBJ)$@


test_data: $(TESTE_DATA)
	$(CC) $(TESTE_DATA) -o $(BIN)test_data

test_entry: $(TESTE_ENTRY)
	$(CC) $(TESTE_ENTRY) -o $(BIN)test_entry

test_list: $(TESTE_LIST)
	$(CC) $(TESTE_LIST) -o $(BIN)test_list

test_table: $(TESTE_TABLE)
	$(CC) $(TESTE_TABLE) -o $(BIN)test_table

test_serialization: $(TESTE_SERIALIZATION)
	$(CC) $(TESTE_SERIALIZATION) -o $(BIN)test_serialization


clean:
	rm -f $(BIN)*
	rm -f $(OBJ)*
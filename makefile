CC=gcc
CFLAGS=-W -Wall -ansi -pedantic -std=c99
LDFLAGS=
SERVER=bin/DLSServer
CLIENT=bin/DLSClient

all: $(SERVER) $(CLIENT)

$(SERVER): src/server_DLS.c
	@$(CC) -o $@ $^ $(CFLAGS)
	
$(CLIENT): src/client_DLS.c
	@$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	@rm -rf $(SERVER) $(CLIENT)
	
mrproper: clean


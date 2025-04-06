CC = qcc
CFLAGS = -Vgcc_ntoaarch64le -Wall
LDFLAGS = -lsocket

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

clean:
	rm -f server client

.PHONY: all clean
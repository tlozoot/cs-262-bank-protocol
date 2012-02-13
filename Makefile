CC = gcc -ggdb -Wall -lpthread

all: clean client server

client: 
	$(CC) bankclient.c csapp.c -o client

server:
	$(CC) bankserver.c csapp.c -o server

clean:
	rm -rf client server *.o *.dSYM

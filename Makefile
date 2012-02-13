CC = gcc -ggdb -Wall -std=c99

all: client server

client: csapp.h
	$(CC) -o client bankclient.c csapp.c 

server: csapp.h
	$(CC) -o server bankserver.c csapp.c echo.c

clean:
	rm -rf client server *.o *.dSYM
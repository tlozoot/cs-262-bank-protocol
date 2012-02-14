/*
 * bankclient.c
 * Adapted from code/netp/echoclient.c in CSAPP
 */

#include "csapp.h"
#include "bank.h"

void parse_message(char buf[], protocol_message *msg);

int main(int argc, char **argv) 
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    
    if (argc == 2 || argc == 3) {
        host = argv[1];
        port = (argc == 2) ? DEFAULT_PORT : atoi(argv[2]);
    }
    else {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }    
    
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);
    
    printf("Bank client v.%d connected to %s on port %d\n", VERSION, host, port);
    
    protocol_message *msg = malloc(sizeof(protocol_message));
    msg->version = VERSION;
    
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        parse_message(buf, msg);
        Rio_writen(clientfd, (void *) msg, message_len(msg));
        Rio_readlineb(&rio, buf, MAXLINE);
    }
    
    Close(clientfd); //line:netp:echoclient:close
    free(msg);
    exit(0);
}

void parse_message(char buf[], protocol_message *msg)
{
    msg->opcode = 0x21;
    strncpy(msg->payload, buf, MAX_PAYLOAD_SIZE);
}
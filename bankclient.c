/*
 * bankclient.c
 * Adapted from code/netp/echoclient.c in CSAPP
 */

#include "csapp.h"
#include "bank.h"

void parse_buf(char buf[], protocol_message *msg);

void print_response(protocol_message *response);

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
    
    protocol_message *request = malloc(sizeof(protocol_message));
    request->version = VERSION;
    
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        parse_buf(buf, request);
        Rio_writen(clientfd, (void *) request, message_len(request)); // Send bits to server
        
        Rio_readlineb(&rio, buf, MAXLINE); // Get bits back from server
        print_response((protocol_message *) buf);
    }
    
    Close(clientfd); //line:netp:echoclient:close
    free(request);
    exit(0);
}

void parse_buf(char buf[], protocol_message *msg)
{
    msg->opcode = 0x21;
    strncpy(msg->payload, buf, MAX_PAYLOAD_SIZE);
}

void print_response(protocol_message *response)
{
    printf("Client received a message of length %d bytes\n", (int) message_len(response));
    printf("Opcode: %x\n", response->opcode);
    printf("Payload: %s\n\n", response->payload);
}
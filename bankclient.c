/*
 * bankclient.c
 * Adapted from code/netp/echoclient.c in CSAPP
 */

#include "csapp.h"
#include "bank.h"

int parse_buf(char buf[], msg_t *msg);

void print_response(msg_t *response);

int main(int argc, char **argv) 
{
    int clientfd, port;
    char *host, buf[MAX_LINE];
    
    if (argc == 2 || argc == 3) {
        host = argv[1];
        port = (argc == 2) ? DEFAULT_PORT : atoi(argv[2]);
    }
    else {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }    
    
    clientfd = Open_clientfd(host, port);
    
    printf("Bank client v.%d connected to %s on port %d\n", VERSION, host, port);
    
    msg_t *request = new_msg();
    msg_t *response = new_msg();
    
    printf("> "); fflush(stdout);
    while (Fgets(buf, MAX_LINE, stdin) != NULL) {
        if (parse_buf(buf, request)) {
            Rio_writen(clientfd, (void *) request, sizeof(msg_t)); // Send bits to server
            Rio_readn(clientfd, (void *) response, sizeof(msg_t)); // Get bits back from server
            print_response(response);    
        }
        
        clear_msg(request);
        clear_msg(response);        
        printf("> "); fflush(stdout);
    }
    
    Close(clientfd); 
    free(request);
    free(response);
    exit(0);
}

int parse_buf(char buf[], msg_t *msg)
{
    if (strncmp(buf, "create", 6) == 0) {
        msg->opcode = 0x10;
        msg->amt = atoi(buf + 7);
        return 1;
    }
    
    else if (strncmp(buf, "deposit", 7) == 0) {
        msg->opcode = 0x25;
        return 1;
    }
    
    else if (strncmp(buf, "withdraw", 8) == 0) {
        msg->opcode = 0x30;
        return 1;
    }
    
    else if (strncmp(buf, "balance", 7) == 0) {
        msg->opcode = 0x40;
        return 1;
    }
    
    else if (strncmp(buf, "close", 5) == 0) {
        msg->opcode = 0x50;
        return 1;
    }
    
    else if (strncmp(buf, "op", 2) == 0) {
        int code = atoi(buf + 3);
        msg->opcode = code;
        return 1;
    }
    
    else {
        printf("Command not understood.\n");
        return 0;
    }
}

void print_response(msg_t *response)
{   
    if (response->version != VERSION) {
        printf("Unknown server version %d\n", response->version);
        return;
    }
    
    if (response->opcode == 0x11) {
        printf("Server successfully created account %d with initial deposit %lld\n", 
                response->acct, response->amt);
    }
    else if (response->opcode == 0x21) {
        printf("Server successfully desposited %lld into account %d\n", response->amt, 
                response->acct);
    }
    else if (response->opcode == 0x31) {
        printf("Server successfully withdrew %lld from account %d\n", response->amt, 
                response->acct);
    }
    else if (response->opcode == 0x41) {
        printf("Account %d has a current balance of %lld\n",  response->acct, response->amt);
    }
    else if (response->opcode == 0x51) {
        printf("Successfully closed account %d\n", response->acct);
    }
    else if (response->opcode == 0x90) {
        printf("Server exception (Unknown version): '%s'\n", response->error);
    }
    else if (response->opcode == 0x91) {
        printf("Server exception (Unknown operation code): '%s'\n", response->error);
    }
    else if (response->opcode == 0x92) {    
        printf("Server exception (Invalid payload): '%s'\n", response->error);
    }
    else if (response->opcode == 0x93) {
        printf("Server exception (Insufficient funds): '%s'\n", response->error);
    }
    else if (response->opcode == 0x94) {
        printf("Server exception (No such account): '%s'\n", response->error);
    }
    else if (response->opcode == 0x95) {
        printf("Server exception (Request denied): '%s'\n", response->error);
    }
    else if (response->opcode == 0x96) {
        printf("Server exception (Internal server error): '%s'\n", response->error);
    }
    
    else {
        printf("Didn't understand server response\n");
    }
    
    fflush(stdout);
}
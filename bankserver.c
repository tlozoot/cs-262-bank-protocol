/* 
 * bankserver.c
 * Adapted from code/netp/echoserveri.c in CSAPP.
 */ 
 
#include "csapp.h"
#include "bank.h"

void handle_connection(int connfd, struct hostent *hp, char *haddrp);
void raise_exception(int connfd, unsigned short opcode, msg_t *request, char *error_msg);

int main(int argc, char **argv) 
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
        
    // Make sure arguments are kosher
    if (argc == 1 || argc == 2) {
        port = (argc == 1) ? DEFAULT_PORT : atoi(argv[1]);
    } else {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    
    // Start listening
    listenfd = Open_listenfd(port);
    printf("Starting banking server v.%d on port %d\n", VERSION, port);
    srand(time(NULL));
    
    // Loop to accept clients
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        
        /* determine the domain name and IP address of the client */
        hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
               sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);
        
        printf("Accepted connection from %s (%s)\n", hp->h_name, haddrp);
        handle_connection(connfd, hp, haddrp);
        Close(connfd);
        printf("Closed connection from %s (%s)\n", hp->h_name, haddrp);
    }
    
    exit(0);
}

void handle_connection(int connfd, struct hostent *hp, char *haddrp) 
{
    size_t len; 
    msg_t *request = new_msg();
    msg_t *response = new_msg();
    
    // Loop to handle multiple messages from one client. 
    while((len = Rio_readn(connfd, (void *) request, sizeof(msg_t))) != 0) {
        // printf("request: "); hex_dump(request);
        
        if (request->opcode == 0x10) {
            unsigned long long amount = request->amt;
            printf("%s (%s) - create %lld\n", hp->h_name, haddrp, amount);

            unsigned int account = rand(); // create_account(amount);
            response->opcode = 0x11;
            response->amt = amount;
            response->acct = account;

            printf(" * created account %d with intial deposit %lld\n", account, amount);
            fflush(stdout);
        }
        else if (request->opcode == 0x20) {
            
        }
        else if (request->opcode == 0x30) {
        
        }
        else if (request->opcode == 0x40) {
        
        }
        else if (request->opcode == 0x50) {
        
        }
        else {
            char error[MAX_LINE];
            sprintf(error, "Unknown opcode: %d", request->opcode);
            raise_exception(connfd, 0x91, request, error);
            printf("%s (%s) - [unknown opcode]\n", hp->h_name, haddrp);
        }
        
        // send bits back to client
        Rio_writen(connfd, (void *) response, sizeof(msg_t));
        clear_msg(request);
        clear_msg(response);
    }    
    
    free(request);
    free(response);
}

void raise_exception(int connfd, unsigned short opcode, msg_t *request, char *error_msg)
{
    msg_t *response = new_msg();
    response->opcode = opcode;
    
    Rio_writen(connfd, (void *) response, sizeof(msg_t));
    free(response);
}

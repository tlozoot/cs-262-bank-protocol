/* 
 * bankserver.c
 * Adapted from code/netp/echoserveri.c in CSAPP.
 */ 
 
#include "csapp.h"
#include "bank.h"

void handle_connection(int connfd);

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
    
    // Loop to accept multiple clients
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        
        /* determine the domain name and IP address of the client */
        hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
               sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);
        
        printf("Accepted connection from %s (%s)\n", hp->h_name, haddrp);
        handle_connection(connfd);
        Close(connfd);
        printf("Closed connection from %s (%s)\n", hp->h_name, haddrp);
    }
    
    exit(0);
}

void handle_connection(int connfd) 
{
    size_t n; 
    protocol_message *msg;
    rio_t rio;
    
    Rio_readinitb(&rio, connfd);
    
    // Loop to handle multiple messages from one client. 
    while((n = Rio_readlineb(&rio, (void *) msg, MAXLINE)) != 0) { //line:netp:echo:eof
        printf("Server received a message of length %d bytes\n", (int) n);
        printf("Opcode: %x\n", msg->opcode);
        printf("Payload: %s\n\n", msg->payload);
        
        Rio_writen(connfd, (void *) msg, message_len(msg)); // send bits back to client
    }    
}

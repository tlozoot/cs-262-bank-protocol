/* 
 * bankserver.c
 * Adapted from code/netp/echoserveri.c in CSAPP.
 */ 
 
#include "csapp.h"
#include "bank.h"

#define MAX_ACCTS 32

typedef struct {
    int open;
    unsigned long long balance;
} account;

account accounts[MAX_ACCTS];

int get_account(void);
void handle_connection(int connfd, struct hostent *hp, char *haddrp);

int main(int argc, char **argv) 
{
    // initialze accounts 
    int i;
    for (i = 0; i < MAX_ACCTS; i++) {
        accounts[i].open = 0;
        accounts[i].balance = 0;
    }
    
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

int open_account(void)
{
    int i;
    for (i = 0; i < MAX_ACCTS; i++) {
        if (! accounts[i].open) {
            accounts[i].open = 1;
            return i;
        }
    }
    
    return -1; // if no open accounts
}

void handle_connection(int connfd, struct hostent *hp, char *haddrp) 
{
    size_t len; 
    msg_t *request = new_msg();
    msg_t *response = new_msg();
    
    // Loop to handle multiple messages from one client. 
    while ((len = Rio_readn(connfd, (void *) request, sizeof(msg_t))) != 0)
    {             
        if (request->version != VERSION) {
            printf("%s (%s) - [version %d]\n", hp->h_name, haddrp, request->version);
            response->opcode = 0x90;
            sprintf(response->error, "Incompatible version number %d (expected version %d)", request->version, VERSION);
            printf("# Error 0x91 (Unknown version): %s\n", response->error);   
        }
        
        else if (request->opcode == 0x10) {
            printf("%s (%s) - create %llu\n", hp->h_name, haddrp, request->amt);

            int account = open_account();
            accounts[account].balance = request->amt;
            
            response->amt = request->amt;
            response->acct = (unsigned int) account;
            
            if (account == -1) {
                response->opcode = 0x97;
                sprintf(response->error, "The maxnimum number of accounts is %u", MAX_ACCTS);
                printf("Error 0x95 (Too many accounts): %s\n", response->error);
            }
            else {
                response->opcode = 0x11;
                printf("# Created account %u with intial deposit %llu\n", account, response->amt);                
            }
        }
        
        else if (request->opcode == 0x20) {
            printf("%s (%s) - deposit %u %llu\n", hp->h_name, haddrp, request->acct, request->amt);
            
            response->acct = request->acct;
            
            if ((request->acct < MAX_ACCTS) && accounts[request->acct].open)
            {
                accounts[request->acct].balance += request->amt;
                response->amt = accounts[request->acct].balance;
                response->opcode = 0x21;
                printf("# Deposited %llu into account %u (new balance: %llu) \n", request->amt, request->acct, response->amt);                
            }
            
            else {
                response->opcode = 0x94;
                sprintf(response->error, "Account %u does not exist", request->acct);
                printf("# Error 0x94 (No such account): %s\n", response->error);
            }       
        }
        
        else if (request->opcode == 0x30) {
            printf("%s (%s) - withdraw %u %llu\n", hp->h_name, haddrp, request->acct, request->amt);
            
            response->acct = request->acct;
            
            if ((request->acct < MAX_ACCTS) && accounts[request->acct].open)
            {    
                if (request->amt > accounts[request->acct].balance) {
                    response->opcode = 0x93;
                    response->amt = accounts[request->acct].balance;
                    sprintf(response->error, "Account %u has a balance of %llu, but %llu was requested for withdrawl",
                            request->acct, accounts[request->acct].balance, request->amt);
                    printf("# Error 0x93 (Insufficient funds): %s\n", response->error);
                }
                else { 
                    accounts[request->acct].balance -= request->amt;
                    response->amt = accounts[request->acct].balance;
                    response->opcode = 0x31;
                    printf("# Withdrew %llu from account %u (new balance: %llu) \n", request->amt, request->acct, response->amt);                
                }
            }
            
            else {
                response->opcode = 0x94;
                sprintf(response->error, "Account %u does not exist", request->acct);
                printf("# Error 0x94 (No such account): %s\n", response->error);
            }       
        
        }
        else if (request->opcode == 0x40) {
            printf("%s (%s) - balance %u\n", hp->h_name, haddrp, request->acct);
            
            response->acct = request->acct;
            
            if ((request->acct < MAX_ACCTS) && accounts[request->acct].open)
            {
                response->amt = accounts[request->acct].balance;                
                response->opcode = 0x41;
                printf("# Account %u has balance %llu\n", request->acct, response->amt);                
            }
            
            else {
                response->opcode = 0x94;
                sprintf(response->error, "Account %u does not exist", request->acct);
                printf("# Error 0x94 (No such account): %s\n", response->error);
            }       
        }
        
        else if (request->opcode == 0x50) {
            printf("%s (%s) - balance %u %llu\n", hp->h_name, haddrp, request->acct, request->amt);
            
            response->acct = request->acct;
            
            if ((request->acct < MAX_ACCTS) && accounts[request->acct].open)
            {
                response->amt = accounts[request->acct].balance;
                accounts[request->acct].balance = 0;
                accounts[request->acct].open = 0;
                response->opcode = 0x51;
                printf("# Account %u was closed with final balance %llu\n", request->acct, response->amt);                
            }
            else {
                response->opcode = 0x94;
                sprintf(response->error, "Account %u does not exist", request->acct);
                printf("# Error 0x94 (No such account): %s\n", response->error);
            }       
        }
        
        else {
            printf("%s (%s) - [opcode %d]\n", hp->h_name, haddrp, request->opcode);
            response->opcode = 0x91;
            sprintf(response->error, "Opcode 0x%02x was not recognized", request->opcode);
            printf("# Error 0x91 (Unknown opcode): %s\n", response->error);
        }
        
        // send bits back to client
        Rio_writen(connfd, (void *) response, sizeof(msg_t));
        clear_msg(request);
        clear_msg(response);
    }    
    
    free(request);
    free(response);
}

/* 
 * bank.h
 * Defines nessage structures and other things common to client and server
 *
 */
 
#include <stdint.h>

#define VERSION 1
#define MAX_MSG 65536
#define MESSAGE_OVERHEAD 16
#define MAX_ERR MAX_MSG - MESSAGE_OVERHEAD

#define DEFAULT_PORT 8372
#define DEFAULT_HOST "localhost"

// typedef (unsigned long long) amt_t;
// typedef (unsigned long) acct_t;

typedef struct {
    unsigned short version;
    unsigned short opcode;
    unsigned int acct;
    unsigned long long amt;
    char error[MAX_ERR];
} msg_t;

size_t msg_len(msg_t *msg)
{
    return strlen(msg->error) + MESSAGE_OVERHEAD;
}

void hex_dump(msg_t *msg)
{
    int i;
    int len = msg_len(msg) / sizeof(unsigned short);
    for (i = 0; i < len; i++) {
        printf("%04x ", *((unsigned short *) msg + i));
    }
    printf("\n");
}

void clear_msg(msg_t *msg)
{
    msg->version = 0x1;
    msg->opcode = 0;
    msg->acct = 0;
    msg->amt = 0;
    msg->error[0] = '\0';
}

msg_t *new_msg()
{
    msg_t *msg = malloc(sizeof(msg_t));
    clear_msg(msg);
    return msg;
}

/* 
 * bank.h
 * Defines nessage structures and other things common to client and server
 *
 */
 
#include <stdint.h>

#define VERSION 1
#define MAX_MESSAGE_SIZE 65536
#define MESSAGE_OVERHEAD 4
#define MAX_PAYLOAD_SIZE MAX_MESSAGE_SIZE - MESSAGE_OVERHEAD

#define DEFAULT_PORT 8372
#define DEFAULT_HOST "localhost"

typedef struct {
    short unsigned version;
    short unsigned opcode;
    char payload[MAX_PAYLOAD_SIZE];
} protocol_message;

size_t message_len(protocol_message *msg)
{
    return strlen(msg->payload) + MESSAGE_OVERHEAD;
}
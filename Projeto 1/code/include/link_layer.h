// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include "state.h"
#include "alarm.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#define BIT(n) (1 << n)

// Message
#define CTRL_S(s) ((s == 0) ? 0 : BIT(6))
#define BCC(a, b) ((a) ^ (b))
#define EMITTER_ADDR 0x03

#define CTRL_SET 0x03
#define CTRL_DISC 0x0B
#define CTRL_UA 0x07
#define CTRL_RR(r) ((r == 0) ? 0x05 : 0x85)
#define CTRL_REJ(r) ((r == 0) ? 0x01 : 0x81)

#define MAX_PAYLOAD_SIZE 1000

#define NO_RESPONSE -1

#define _POSIX_SOURCE 1 
#define ALARM_TIMEOUT 3
#define MAX_RETRIES 3
#define MAX_BUFFER_SIZE (MAX_PAYLOAD_SIZE*2)

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
} LinkLayer;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer connectionParameters);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(const unsigned char *buf, int bufSize);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(unsigned char *packet);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int showStatistics);

#endif // _LINK_LAYER_H_

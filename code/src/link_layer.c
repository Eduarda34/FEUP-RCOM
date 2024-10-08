// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"
#include "state.h"
#include "message.h"

#include <iostream>
#include <string.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

// Global Variables
unsigned char byte;
int timeout = 0;
int nretransmissions = 0;
int fd;

int startTransmissor (int fd) {return send_s_frame(fd, ADDR, 0X03, R_UA);}

int startReceiver (int fd) {
    unsigned char message[5];

    if (read_message(fd, message, 5, COMMAND_SET) < 0) return -1;

    return send_s_frame(fd, ADDR, 0x07, NO_RESP);
}

//LLPOPEN
int llopen(LinkLayer connectionParameters) {
    fd = openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate);
    struct termios newtio;
    

    // Check if openSerialPort has error
    if (fd < 0) {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    


    return 1;
}

int llwrite(const unsigned char *buf, int bufSize) {
    // TODO

    return 0;
}

int llread(unsigned char *packet) {
    // TODO

    return 0;
}

int llclose(int showStatistics) {
    // TODO

    int clstat = closeSerialPort();
    return clstat;
}
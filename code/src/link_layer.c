// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"
#include "state.h"

#include <iostream>
#include <string.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

// Global Variables
unsigned char byte;
int timeout = 0;
int nretransmissions = 0;



int llopen(LinkLayer connectionParameters) {
    int fd = openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate);

    // Check if openSerialPort has error
    if (fd < 0) {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    

    // TODO

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
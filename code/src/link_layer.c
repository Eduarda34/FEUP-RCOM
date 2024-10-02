// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"
#include "frame_functions.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

// Global Variables
unsigned char byte;
int timeout = 0;
int nretransmissions = 0;



////////////////////////////////////////////////
// LLOPEN
// Returns "1" in case of success, or "-1" in case of error.
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd = openSerialPort(connectionParameters.serialPort,
                       connectionParameters.baudRate);

    // Check if openSerialPort has error
    if (fd < 0)
    {
        return -1;
    }


    timeout = connectionParameters.timeout;
    nretransmissions = connectionParameters.nRetransmissions;

    switch(connectionParameters.role) {
        // Check only the Receiver stuff
        case LlRx:
            // while 
            if(read(fd,&byte, 1) > 0) {
                if(byte ==  0x7E) {
                    break;
                }
                if(connectionParameters.timeout==0) break;
            }
            
        // Check the Transmitter and Receiver
        case LlTx:
            // while 
            if(connectionParameters.timeout==0) break;
            if(send_frame(fd,DDR,0X0(fd) < 0)
            {
                return -1;
            }
        default:
            return -1;
            break;

    }

    // TODO

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
// Returns number of written chars, or "-1" in case of error.
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
// Returns number of written chars, or "-1" in case of error.
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
// Returns "1" in case of success, or "-1" in case of error.
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    int clstat = closeSerialPort();
    return clstat;
}
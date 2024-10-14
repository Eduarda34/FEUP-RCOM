// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"
#include "state.h"
#include "message.h"

#include <string.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

// Global Variables
unsigned char byte;
int timeout = 0;
int nretransmissions = 0;
int fd;

int startTransmissor (int fd) {
    return send_s_frame(fd, ADDR, 0X03, R_UA);
}

int startReceiver (int fd) {
    unsigned char message[5];
    if (read_message(fd, message, 5, COMMAND_SET) < 0) return -1;
    return send_s_frame(fd, ADDR, 0x07, NO_RESP);
}

//LLPOPEN
int llopen(LinkLayer connectionParameters) {
    
    fd = openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate);

    // Check if openSerialPort has error
    if (fd < 0) {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    memset(&newtio, 0, sizeof(newtio));

    // Set baud rate
    cfsetispeed(&newtio, connectionParameters.baudRate);
    cfsetospeed(&newtio, connectionParameters.baudRate);

    // Configure 8N1 mode (8 data bits, no parity, 1 stop bit)
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;

    // No hardware flow control
    newtio.c_cflag &= ~CRTSCTS;

    // Disable canonical mode, echoing, and signal characters
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Disable software flow control
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Set timeouts
    newtio.c_cc[VMIN] = 0;  // No minimum characters for non-blocking read
    newtio.c_cc[VTIME] = connectionParameters.timeout * 10;  // Set timeout in deciseconds

    // Apply the settings to the serial port
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    TRANSMITTER
    // Start connection process based on role
    if (TRANSMITTER) {
        // Transmitter: Send a SET command and wait for UA (Unnumbered Acknowledgment)
        if (startTransmissor(fd) < 0) {
            perror("Error starting transmitter");
            return -1;
        }
    } else if (connectionParameters.role == LlRx) {
        // Receiver: Wait for a SET command and respond with UA
        if (startReceiver(fd) < 0) {
            perror("Error starting receiver");
            return -1;
        }
    } else {
        perror("Unknown role");
        return -1;
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
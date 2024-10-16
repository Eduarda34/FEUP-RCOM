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
struct termios oldtio; 

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
    
    // Open the serial port with specified parameters
    fd = openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate);
    
    // Check if openSerialPort has encountered an error
    if (fd < 0) {
        fprintf(stderr, "Error opening serial port: %s\n", connectionParameters.serialPort);
        perror("openSerialPort");
        return -1;
    }

    struct termios newtio;

    // Get current port settings for backup
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));  // Clear the newtio structure

    // Set input and output baud rates
    cfsetispeed(&newtio, connectionParameters.baudRate);
    cfsetospeed(&newtio, connectionParameters.baudRate);

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Set minimum characters for read to 0 and timeouts in deciseconds
    newtio.c_cc[VMIN] = 0;
    switch (connectionParameters.role) {
        case TRANSMITTER:
            newtio.c_cc[VMIN] = 0;                             // Non-blocking read
            newtio.c_cc[VTIME] = connectionParameters.timeout; // Set timeout in deciseconds
            break;

        case RECEIVER:
            newtio.c_cc[VMIN] = 1; // Blocking read until at least 1 char is received
            newtio.c_cc[VTIME] = 0; // No timeout, wait indefinitely for a character
            break;

        default:
            fprintf(stderr, "Unknown role: %d\n", connectionParameters.role);
            return -1;
    }

    // Apply the new settings immediately
    tcflush(fd, TCIOFLUSH);
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        return -1;
    }

    printf("********** New structure **********\n");

    (void)signal(SIGALRM, alarm_handler);

    // Determine role: Transmitter or Receiver
    switch (connectionParameters.role) {
        case TRANSMITTER:
            set_role(TRANSMITTER);
            if (startTransmissor(fd) < 0) {
                perror("Error starting transmitter");
                return -1;
            }
            break;

        case RECEIVER:
            set_role(RECEIVER);
            if (startTransmissor(fd) < 0) {
                perror("Error starting transmitter");
                return -1;
            }
            break;

        default:
            // Handle unknown roles
            fprintf(stderr, "Unknown role: %d\n", connectionParameters.role);
            return -1;

    }

    printf("********** Connection Successfully **********\n");
    return fd;
}


int llwrite(const unsigned char *buf, int bufSize) {
    int bytes;
    int sequence_number = 0;
    // Send the I-frame
    bytes = send_i_frame(fd, buf, bufSize, sequence_number);

    // Check if sending the frame was successful
    if (bytes == -1) {
        perror("Error sending I-frame");
        return -1;
    }

    printf("llwrite: %d bytes written\n", bytes);

    sequence_number ^= 0x01;

    printf("********** Bytes Successfully written **********\n");
    return bytes;
}


int llread(unsigned char *packet) {
    // TODO

    return 0;
}

int llclose(int showStatistics) {
    // Handle connection closure based on the current role
    switch (get_curr_role()) {
        case TRANSMITTER:
            if (close_transmissor(fd) < 0) {
                perror("Error closing TRANSMITTER");
                return -1;
            }
            break;

        case RECEIVER:
            if (close_receiver(fd) < 0) {
                perror("Error closing RECEIVER");
                return -1;
            }
            break;

        default:
            // In case of an unknown role, return error
            fprintf(stderr, "Unknown role: %d\n", get_curr_role());
            return -1;
    }

    // Indicate the start of the connection closing process
    printf("Closing connection...\n");

    // Optional: If showStatistics is needed, you can print here (you can expand this later)
    if (showStatistics) {
        printf("Statistics: Data transmitted successfully.\n");
    }

    // Delay to ensure the closure has time to finish
    sleep(1);

    // Restore the original port settings (using the stored oldtio)
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr failed");
        return -1;
    }

    // Close the serial port file descriptor
    if (close(fd) < 0) {
        perror("Error closing file descriptor");
        return -1;
    }

    // Return success
    return 1;
}

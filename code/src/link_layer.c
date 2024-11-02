// Link layer protocol implementation
#include "link_layer.h"


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

int closeTransmissor(int fd) {
    // Send DISC frame to the receiver to initiate disconnection
    if (send_s_frame(fd, ADDR, 0x0B, COMMAND_DISC) < 0) {
        perror("Error sending DISC frame\n");
        return -1;
    }

    printf("DISC frame sent to receiver\n");

    // Wait for DISC frame from the receiver (acknowledgment)
    unsigned char message[5];
    if (read_message(fd, message, 5, COMMAND_DISC) < 0) {
        perror("Error receiving DISC frame from receiver\n");
        return -1;
    }

    printf("DISC frame received from receiver\n");

    // Send UA frame to acknowledge the connection termination
    if (send_s_frame(fd, ADDR, 0x03, R_UA) < 0) {
        perror("Error sending UA frame\n");
        return -1;
    }

    printf("UA frame sent, connection closed\n");

    return 0;
}

int closeReceiver(int fd) {
    // Wait for DISC frame from the transmitter
    unsigned char message[5];
    if (read_message(fd, message, 5, COMMAND_DISC) < 0) {
        perror("Error receiving DISC frame from transmitter\n");
        return -1;
    }

    printf("DISC frame received from transmitter\n");

    // Send DISC frame to the transmitter to acknowledge disconnection
    if (send_s_frame(fd, ADDR, 0x0B, COMMAND_DISC) < 0) {
        perror("Error sending DISC frame\n");
        return -1;
    }

    printf("DISC frame sent to transmitter\n");

    // Wait for UA frame from the transmitter to confirm connection termination
    if (read_message(fd, message, 5, R_UA) < 0) {
        perror("Error receiving UA frame from transmitter\n");
        return -1;
    }

    printf("UA frame received, connection closed\n");

    return 0;
}


int openTransmissor () {
    return 0;
}


int openReceiver () {
    return 0;
}

int llopen(LinkLayer connectionParameters) {
   

    struct termios newtio;

    // Get current port settings for backup
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr\n");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));  // Clear the newtio structure

    // Set input and output baud rates
    // cfsetispeed(&newtio, connectionParameters.baudRate);
    // cfsetospeed(&newtio, connectionParameters.baudRate);

    // newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    // newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

    // // Set minimum characters for read to 0 and timeouts in deciseconds
    // newtio.c_cc[VMIN] = 0;
    // switch (connectionParameters.role) {
    //     case TRANSMITTER:
    //         newtio.c_cc[VMIN] = 0;                             // Non-blocking read
    //         newtio.c_cc[VTIME] = connectionParameters.timeout; // Set timeout in deciseconds
    //         break;

    //     case RECEIVER:
    //         newtio.c_cc[VMIN] = 1; // Blocking read until at least 1 char is received
    //         newtio.c_cc[VTIME] = 0; // No timeout, wait indefinitely for a character
    //         break;

    //     default:
    //         fprintf(stderr, "Unknown role: %d\n", connectionParameters.role);
    //         return -1;
    // }

    // Apply the new settings immediately
    //tcflush(fd, TCIOFLUSH);
    // if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    //     perror("tcsetattr\n");
    //     return -1;
    // }

    printf("********** New structure **********\n");

    //(void)signal(SIGALRM, alarm_handler);

    switch (connectionParameters.role) {
        case LlTx:
            printf("Start Transmissor\n");
            set_role(TRANSMITTER);
            //(void)signal(SIGALRM, alarm_handler);
            if (startTransmissor(fd) < 0) {
                perror("Error starting transmitter\n");
                return -1;
            }
            
            break;

        case LlRx:
            set_role(RECEIVER);
            //(void)signal(SIGALRM, alarm_handler);
            if (startReceiver(fd) < 0) {
                perror("Error starting transmitter\n");
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
        perror("Error sending I-frame\n");
        return -1;
    }

    printf("llwrite: %d bytes written\n", bytes);

    sequence_number ^= 0x01;

    printf("********** Bytes Successfully written **********\n");
    return bytes;
}


int llread(unsigned char *packet) {
    unsigned char stuffed_msg[MSG_MAX_SIZE * 2]; // To hold the received stuffed message
    unsigned char destuffed_msg[MSG_MAX_SIZE];   // To hold the destuffed message
    int bytes;

    // Attempt to read the message from the serial port
    bytes = read_message(fd, stuffed_msg, sizeof(stuffed_msg), COMMAND_DATA);

    // Check for errors during reading
    if (bytes == -1) {
        perror("Error reading message\n");
        return -1;
    }

    printf("llread: %d bytes read\n", bytes);

    // Perform message de-stuffing
    int destuffed_len = msg_destuff(stuffed_msg, 4, bytes, destuffed_msg);

    // Ensure the message size is valid
    if (destuffed_len < 6) { // At least FLAG, ADDR, CONTROL, BCC1, BCC2, FLAG
        printf("llread: Invalid message length\n");
        return -1;
    }

    // Verify the BCC1 for header (ADDR ^ CONTROL)
    if ((destuffed_msg[1] ^ destuffed_msg[2]) != destuffed_msg[3]) {
        printf("llread: BCC1 verification failed\n");
        return -1;
    }


    // Calculate BCC2 for the data field
    uint8_t received_bcc2 = destuffed_msg[destuffed_len - 2]; // Second-to-last byte is BCC2
    uint8_t calculated_bcc2 = generate_bcc2(destuffed_msg + 4, destuffed_len - 6); // Skip FLAG, ADDR, CONTROL, BCC1, FLAG

    // Verify the BCC2 for the data field
    if (received_bcc2 != calculated_bcc2) {
        printf("llread: BCC2 verification failed\n");
        return -1;
    }

    // If everything is correct, copy the data to the provided packet buffer
    memcpy(packet, destuffed_msg + 4, destuffed_len - 6); // Copy only the data, skipping FLAGs, ADDR, CONTROL, BCCs

    printf("llread: Message successfully read and verified\n");
    return destuffed_len - 6; // Return the number of data bytes
}

int llclose(int showStatistics) {
    // Handle connection closure based on the current role
    switch (get_curr_role()) {
        case TRANSMITTER:
            if (closeTransmissor(fd) < 0) {
                perror("Error closing TRANSMITTER\n");
                return -1;
            }
            break;

        case RECEIVER:
            if (closeReceiver (fd) < 0) {
                perror("Error closing RECEIVER\n");
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
        perror("tcsetattr failed\n");
        return -1;
    }

    // Close the serial port file descriptor
    if (close(fd) < 0) {
        perror("Error closing file descriptor\n");
        return -1;
    }

    // Return success
    return 1;
}

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
static uint8_t sequence_number = 0;

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

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    int bytes = send_i_frame(fd, buf, bufSize, sequence_number);
    if (bytes == -1) {
        perror("llwrite: Error sending I-frame");
        exit(EXIT_FAILURE);
    }

    printf("-----------------------------------\n");
    printf("Successfully written %d bytes\n", bytes);
    
    sequence_number ^= 0x01; 
    return bytes;
}


////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    // ALLOC BUFFER - MESSAGE
    unsigned char *message_buffer = (unsigned char *)malloc(MSG_MAX_SIZE * 2);
    if (message_buffer == NULL) {
        perror("Memory allocation for message buffer failed");
        return -1;
    }

    // READ MESSAGE
    int bytes_read = read_message(fd, message_buffer, MSG_MAX_SIZE * 2, COMMAND_DATA);
    if (bytes_read < 0) {
        free(message_buffer);
        return -1; 
    }

    // ALLOC BUFFER - DESTUFFED MESSAGE
    uint8_t *destuffed_message = (uint8_t *)malloc(MSG_MAX_SIZE);
    if (destuffed_message == NULL) {
        perror("Memory allocation for destuffed message failed");
        free(message_buffer);
        return -1;
    }

    // DESTUFFED MESSAGE
    int destuffed_message_size = msg_destuff(message_buffer, 4, bytes_read, destuffed_message);
    free(message_buffer); 

    if (destuffed_message_size < 0) {
        free(destuffed_message);
        fprintf(stderr, "Message destuffing failed\n");
        return -1;
    }

    // EXTRACT & GENERATE BCC2
    unsigned char received_bcc2 = destuffed_message[destuffed_message_size - 2];
    unsigned char expected_bcc2 = generate_bcc2(destuffed_message + 4, destuffed_message_size - 6);

    // CHECK IF BCC2 RECEIVED AND BCC2 EXPECTED EQUA
    if (received_bcc2 == expected_bcc2) {
        // VALIDATE SEQUENCE NUMBER
        if ((get_control() == 0x00 && sequence_number == 0) || (get_control() == 0x40 && sequence_number == 1)) {
            send_s_frame(fd, ADDR, 0x05 | ((sequence_number ^ 0x01) << 7), NO_RESP);
            memcpy(packet, destuffed_message + 4, destuffed_message_size - 6);
            free(destuffed_message);
            sequence_number ^= 0x01; // TOGGLE SEQUENCE NUMBER
            return destuffed_message_size - 6; // RETURN SIZE OF DATA
        } else {
            // DUPLICATE DATA PACKET RECEIVED
            send_s_frame(fd, ADDR, 0x05 | (sequence_number << 7), NO_RESP);
            free(destuffed_message);
            return -1; 
        }
    }

    // BCC2 MISMATCH HANDLING
    send_s_frame(fd, ADDR, 0x01 | ((sequence_number ^ 0x01) << 7), NO_RESP);
    free(destuffed_message);
    return -1; 
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



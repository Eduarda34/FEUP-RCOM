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
// LLWRITE                                   ///
////////////////////////////////////////////////
int stuff_message(uint8_t *buffer, int start, int msg_size, uint8_t *stuffed_msg)
{
    int i = 0;
    // Copy header without stuffing
    for (int j = 0; j < start; ++j, ++i)
        stuffed_msg[i] = buffer[j];
    // Stuffing
    for (int j = start; j < msg_size; ++j)
    {
        if (buffer[j] == FLAG || buffer[j] == ESCAPE) {
            stuffed_msg[i++] = ESCAPE;
            stuffed_msg[i++] = buffer[j] ^ 0x20;
        } else {
            stuffed_msg[i++] = buffer[j];
        }
    }
    return i;
}

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
// LLREAD                                    ///
////////////////////////////////////////////////
int destuff_message(uint8_t *buffer, int start, int msg_size, uint8_t *destuffed_msg)
{
    int i = 0;
    for (int j = 0; j < start; ++j, ++i) {
        destuffed_msg[i] = buffer[j];
    }
    for (int j = start; j < msg_size; j++) {
        if (buffer[j] == ESCAPE) {
            destuffed_msg[i] = buffer[j + 1] ^ 0x20;
            j++;
            i++;
        } else {
            destuffed_msg[i] = buffer[j];
            i++;
        }
    }
    return i;
}

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
    int destuffed_message_size = destuff_message(message_buffer, 4, bytes_read, destuffed_message);
    free(message_buffer); 

    if (destuffed_message_size < 0) {
        free(destuffed_message);
        fprintf(stderr, "Message destuffing failed\n");
        return -1;
    }

     // EXTRACT  BCC2
    unsigned char received_bcc2 = destuffed_message[destuffed_message_size - 2];

    // GENERATE BCC2 
    unsigned char expected_bcc2 = destuffed_message[4]; // Initialize with the first byte of data
    for (int i = 5; i < destuffed_message_size - 2; ++i) { 
        expected_bcc2 ^= destuffed_message[i];
    }


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

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int close_receiver(int fd) {
    printf("Disconnecting receiver...\n");
    unsigned char message[5];

    if (read_message(fd, message, sizeof(message), COMMAND_DISC) < 0) {
        fprintf(stderr, "Error reading disconnect message from receiver.\n");
        return -1;
    }

    if (send_s_frame(fd, ADDR, 0x0B, RESPONSE_UA) < 0) {
        fprintf(stderr, "Error sending UA response from receiver.\n");
        return -1;
    }

    return 0; // Indicate success
}

int close_transmitter(int fd) {
    printf("Disconnecting transmitter...\n");

    // Send a disconnect command
    if (send_s_frame(fd, ADDR, 0x0B, COMMAND_DISC) < 0) {
        fprintf(stderr, "Error sending disconnect command from transmitter.\n");
        return -1;
    }

    // Send an Unnumbered Acknowledge (UA) response
    if (send_s_frame(fd, ADDR, 0x07, NO_RESP) < 0) {
        fprintf(stderr, "Error sending UA response from transmitter.\n");
        return -1;
    }

    return 0; // Indicate success
}

int llclose(int showStatistics) {
    int result = 0; 
    
    switch (get_curr_role())
    {
        case TRANSMITTER:
            result = close_transmitter(fd);
            if (result < 0) {
                fprintf(stderr, "Failed to close TRANSMITTER.\n");
                return -1;
            }
            break;

        case RECEIVER:
            result = close_receiver(fd);
            if (result < 0){
                fprintf(stderr, "Failed to close RECEIVER.\n");
                return -1;
            }
            break;
        default:
            fprintf(stderr, "Unknown role during close operation.\n");
            return -1; 
    }

    printf("Closing connection...\n");
    sleep(1); 

    if (tcsetattr(fd, TCSANOW, &oldtioT) == -1)
    {
        perror("Failed to restore port settings");
        exit(EXIT_FAILURE);
    }

    close(fd);
    return 1; 
}
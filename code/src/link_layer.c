// Link layer protocol implementation
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 
#define ALARM_TIMEOUT 3
#define MAX_RETRIES 3
#define MAX_BUFFER_SIZE (MAX_PAYLOAD_SIZE*2)

struct termios oldtioT;
int fd;
static uint8_t sequence_number = 0;



////////////////////////////////////////////////
/// AUX                                      ///
////////////////////////////////////////////////
int read_message(int fd, uint8_t *buf, int buf_size, command response) {
    int bytesRead = 0;
    reset_state();
    set_command(response);

    while (get_curr_state() != FINAL_STATE && !get_alarm_flag() && bytesRead < buf_size) {
        int bytes = read(fd, buf + bytesRead, 1);
        
        if (bytes == -1) {
            perror("Error reading from file descriptor");
            return -1;
        } else if (bytes == 0) {
            printf("No more data available to read.\n");
            return -1;
        }

        update_state(buf[bytesRead]);
        bytesRead++;
    }

    if (get_curr_state() != FINAL_STATE) {
        if (get_alarm_flag()) {
            printf("Failed to get response: Timeout occurred.\n");
        } else {
            printf("Failed to get response: Buffer limit reached before final state.\n");
        }
        return -1;
    }

    return bytesRead;
}

int send_message(int fd, uint8_t *frame, int msg_size, command response)
{
    int bytesWritten = 0;

    if (response == NO_RESPONSE) { 
        bytesWritten = write(fd, frame, msg_size);
        if (bytesWritten == -1) {
            perror("Write failed");
            return -1;
        }
        return bytesWritten;
    }

    // WRITE RESPONSE
    set_command(response);
    reset_alarm_count();
    reset_state();

    while (get_alarm_count() < MAX_RETRIES && get_curr_state() != FINAL_STATE) {
        set_alarm_flag(FALSE);
        bytesWritten = write(fd, frame, msg_size);

        if (bytesWritten == -1) {
            perror("Write failed");
            return -1;
        }

        printf("Message sent\n");
        
        unsigned char buf[MAX_BUFFER_SIZE] = {0};
        alarm(ALARM_TIMEOUT);
        int bytesRead = 0;

        // LOOP UNTIL FINAL STATE OR TIMEOUT
        while (get_curr_state() != FINAL_STATE && !get_alarm_flag()) {
            if (bytesRead >= MAX_BUFFER_SIZE) {
                printf("Buffer overflow\n");
                break;
            }

            int read_byte = read(fd, buf + bytesRead, 1);
            if (read_byte == -1) {
                perror("Read error");
                return -1;
            } else if (read_byte > 0) {
                update_state(buf[bytesRead]);
                bytesRead++;
            }
        }
    }

    // CHECK IF FINAL STATE REACHED
    if (get_curr_state() != FINAL_STATE) {
        printf("Failed to receive expected response!\n");
        return -1;
    }

    return bytesWritten;
}

uint8_t *create_s_frame_buffer(uint8_t address, uint8_t control) {
    uint8_t *buffer = (uint8_t *)malloc(5);
    if (buffer == NULL) {
        return NULL; // Handle memory allocation failure
    }
    memset(buffer, 0, 5);
    buffer[0] = FLAG;
    buffer[1] = address;
    buffer[2] = control;
    buffer[3] = BCC(buffer[1], buffer[2]);
    buffer[4] = FLAG;
    return buffer;
}

int send_s_frame(int fd, uint8_t address, uint8_t control, command response) {
    uint8_t *buffer = create_s_frame_buffer(address, control);
    if (buffer == NULL) {
        return -1; 
    }
    int bytes = send_message(fd, buffer, 5, response);
    free(buffer);
    return bytes;
}


////////////////////////////////////////////////
/// LLOPEN                                   ///
////////////////////////////////////////////////
int start_transmissor(int fd){
    return send_s_frame(fd, ADDR, 0x03, RESPONSE_UA);
}

int start_receiver(int fd){
    unsigned char message[5];
    if (read_message(fd, message, 5, COMMAND_SET) < 0) return -1;
    return send_s_frame(fd, ADDR, 0x07, NO_RESPONSE);
}

int llopen(LinkLayer connectionParameters) {
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(EXIT_FAILURE);
    }

    struct termios newtio;

    // SAVE PORT SETTINGS
    if (tcgetattr(fd, &oldtioT) == -1)  {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // NEW PORT SETTINGS
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // SET INPUT
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    switch (connectionParameters.role) {
    case TRANSMITTER:
        newtio.c_cc[VMIN] = 0;                             
        newtio.c_cc[VTIME] = connectionParameters.timeout; 
        break;
    case RECEIVER:
        newtio.c_cc[VMIN] = 1; 
        break;
    }

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("Start program. Please wait...\n");

    switch (connectionParameters.role) {
    case TRANSMITTER:
        set_role(TRANSMITTER);
        (void)signal(SIGALRM, alarm_handler);
        if (start_transmissor(fd) < 0) {
            printf("Could not start TRANSMITTER\n");
            return -1;
        }
        break;

    case RECEIVER:
        set_role(RECEIVER);
        (void)signal(SIGALRM, alarm_handler);
        if (start_receiver(fd) < 0) {
            printf("Could not start RECEIVER\n");
            return -1;
        }
        break;
    }

    printf("-----------------------------------\n");
    printf("Connection Successfully\n");
    printf("-----------------------------------\n");

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

int create_stuffed_message(uint8_t *buffer, int msg_len, uint8_t *stuffed_msg) {
    msg_len = stuff_message(buffer, 4, msg_len, stuffed_msg);
    stuffed_msg[msg_len++] = FLAG; 
    return msg_len;
}

uint8_t *create_i_frame_buffer(const uint8_t *data, int data_len, int packet) {
    int msg_len = data_len + 5; // Header (4 bytes) + BCC2 (1 byte)
    uint8_t *buffer = (uint8_t *)malloc(msg_len);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // INIT HEADER
    buffer[0] = FLAG;
    buffer[1] = EMITTER_ADDR;
    buffer[2] = (packet << 6); // Control field with sequence number
    buffer[3] = BCC(buffer[1], buffer[2]);

    // COMPUTE BCC2
    uint8_t bcc2 = 0;
    for (int i = 0; i < data_len; i++) {
        buffer[i + 4] = data[i];
        bcc2 ^= data[i]; 
    }
    buffer[data_len + 4] = bcc2;
    return buffer;
}

int transmit_information_frame(int fd, const uint8_t *data, int data_len, int packet) {
    // Call create_i_frame_buffer to allocate and prepare the I-frame
    uint8_t *buffer = create_i_frame_buffer(data, data_len, packet);
    if (!buffer) {
        return -1; // Handle memory allocation failure
    }

    // Buffer length includes header, data, BCC2, and FLAG
    int msg_len = data_len + 5;
    uint8_t stuffed_msg[msg_len * 2];

    // Create stuffed message
    msg_len = create_stuffed_message(buffer, msg_len, stuffed_msg);
    free(buffer); // Free the allocated I-frame buffer after stuffing

    // Sending process
    for (int w = 0; w < 3; w++) {
        int bytes = send_message(fd, stuffed_msg, msg_len, RESPONSE_REJ);
        if (bytes == -1) {
            printf("Failed to send message correctly\n");
            return -1;
        }

        // Check response
        if ((packet == 0 && get_prev_response() == PA_F1) ||
            (packet == 1 && get_prev_response() == PA_F0)) {
            printf("Positive Acknowledgement :)\n");
            return bytes; // Successful transmission
        }
        if ((packet == 0 && get_prev_response() == REJ_F1) ||
            (packet == 1 && get_prev_response() == REJ_F0)) {
            printf("Invalid message sent and rejected\n");
            continue; // Retry sending
        }
    }

    return -1;
}

int llwrite(const unsigned char *buf, int bufSize)
{
    int bytes = transmit_information_frame(fd, buf, bufSize, sequence_number);
    if (bytes == -1) {
        perror("llwrite: Error sending I-frame");
        exit(EXIT_FAILURE);
    }

    printf("-----------------------------------\n");
    printf("Bytes uccessfully written: %d\n", bytes);
    
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
    unsigned char *message_buffer = (unsigned char *)malloc(MAX_PAYLOAD_SIZE * 2);
    if (message_buffer == NULL) {
        perror("Memory allocation for message buffer failed");
        return -1;
    }

    // READ MESSAGE
    int bytes_read = read_message(fd, message_buffer, MAX_PAYLOAD_SIZE * 2, COMMAND_DATA);
    if (bytes_read < 0) {
        free(message_buffer);
        return -1; 
    }

    // ALLOC BUFFER - DESTUFFED MESSAGE
    uint8_t *destuffed_message = (uint8_t *)malloc(MAX_PAYLOAD_SIZE);
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
            send_s_frame(fd, ADDR, 0x05 | ((sequence_number ^ 0x01) << 7), NO_RESPONSE);
            memcpy(packet, destuffed_message + 4, destuffed_message_size - 6);
            free(destuffed_message);
            sequence_number ^= 0x01; // TOGGLE SEQUENCE NUMBER
            return destuffed_message_size - 6; // RETURN SIZE OF DATA
        } else {
            // DUPLICATE DATA PACKET RECEIVED
            send_s_frame(fd, ADDR, 0x05 | (sequence_number << 7), NO_RESPONSE);
            free(destuffed_message);
            return -1; 
        }
    }

    // BCC2 MISMATCH HANDLING
    send_s_frame(fd, ADDR, 0x01 | ((sequence_number ^ 0x01) << 7), NO_RESPONSE);
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
    if (send_s_frame(fd, ADDR, 0x07, NO_RESPONSE) < 0) {
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
    printf("-----------------------------------\n");
    sleep(1); 

    if (tcsetattr(fd, TCSANOW, &oldtioT) == -1)
    {
        perror("Failed to restore port settings");
        exit(EXIT_FAILURE);
    }

    close(fd);
    return 1; 
}

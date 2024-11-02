// Link layer protocol implementation
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

struct termios oldtioT;
int fd;
static uint8_t sequence_number = 0;


//////////////////////////////////////////////// FALTA CORRIGIR READ AND SEND MESSAGE
//////////////////////////////////////////////// FALTA CORRIGIR llopen MAS VOU DORMIR


////////////////////////////////////////////////
/// AUX                                      ///
////////////////////////////////////////////////
int read_message(int fd, uint8_t *buf, int buf_size, command response){
    int i = 0;
    reset_state();
    set_command(response);
    while (get_curr_state() != FINAL_STATE && !get_alarm_flag()) {
        if (i >= buf_size) break;
        int bytes = read(fd, buf + i, 1);
        if (bytes != -1) update_state(buf[i]);
        i++;
    }
    if (get_curr_state() != FINAL_STATE) {
        printf("Failed to get response!\n");
        return -1;
    }
    return i;
}

int send_message(int fd, uint8_t *frame, int msg_size, command response)
{
    int bytes;
    if (response == NO_RESP) { 
        if ((bytes = write(fd, frame, msg_size)) == -1) {
            printf("Write failed\n");
            return -1;
        }
        return bytes;
    }
    set_command(response);
    reset_alarm_count();
    reset_state();
    while (get_alarm_count() < 3 && get_curr_state() != FINAL_STATE) {
        set_alarm_flag(FALSE);
        if ((bytes = write(fd, frame, msg_size)) == -1) {
            printf("Write failed\n");
            return -1;
        }
        printf("Message sent\n");
        unsigned char buf[MSG_MAX_SIZE*2] = {0};
        alarm(3);
        int i = 0;
        while (get_curr_state() != FINAL_STATE && !get_alarm_flag()) {
            if (i >= (MSG_MAX_SIZE*2)) {
                continue;
            }
            int read_byte = read(fd, buf + i, 1);
            if (read_byte != -1) {
                update_state(buf[i]);
            }
            i++;
        }
    }

    if (get_curr_state() != FINAL_STATE) {
        printf("Failed to get response!\n");
        return -1;
    }

    return bytes;
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
    return send_s_frame(fd, ADDR, 0x07, NO_RESP);
}

int llopen(LinkLayer connectionParameters) {

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)     {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtioT) == -1)  {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    switch (connectionParameters.role)
    {
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
    buffer[1] = ADDR_E;
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
#include "message.h"



int send_s_frame(int fd, uint8_t address, uint8_t control, command response) {
    s_frame frame;
    int bytes;

    // Initialize the S-frame
    init_s_frame(&frame, address, control);

    // Send the frame as a byte array
    if ((bytes = send_message(fd, (uint8_t *)&frame, FRAME_SIZE, response)) == -1) return -1;

    return bytes;
}


int send_i_frame(int fd, const uint8_t *data, int data_len, int packet) {
    i_frame frame;
    int msg_len;
    int bytes;
    uint8_t stuffed_msg[(data_len + 5) * 2];
    int w = 0;

    // Initialize the I-frame fields
    init_i_frame(&frame, data, data_len, packet);

    // Fill the buffer with frame parts and data
    fill_i_frame_buffer(&frame);

    // Perform message stuffing (assuming msg_stuff is implemented)
    msg_len = msg_stuff(frame.buffer, 4, frame.buffer_len, stuffed_msg);
    stuffed_msg[msg_len] = FLAG;
    msg_len++;

    // Try sending the message up to 3 times
    while (w < 3) {
        if ((bytes = send_message(fd, stuffed_msg, msg_len, R_RR_REJ)) == -1) {
            printf("Failed to send message correctly\n");
            free_i_frame(&frame);
            return -1;
        }

        // Check for positive acknowledgment
        if ((packet == 0 && get_prev_response() == RR_1) || (packet == 1 && get_prev_response() == RR_0)) {
            printf("Sent message and received positive acknowledgement\n");
            free_i_frame(&frame);
            return bytes;
        }

        // Handle REJ (negative acknowledgment)
        if ((packet == 0 && get_prev_response() == REJ_1) || (packet == 1 && get_prev_response() == REJ_0)) {
            printf("Invalid message sent and rejected\n");
            w++;
            continue;
        }

        free_i_frame(&frame);
        return -1;
    }

    free_i_frame(&frame);
    return -1;
}


int send_message(int fd, uint8_t *frame, int msg_size, command response) {
    int bytes;

    if (response == NO_RESP) { // no response expected
        if ((bytes = write(fd, frame, msg_size)) == -1) {
            printf("Write failed\n");
            return -1;
        }

        return bytes;
    }

    set_command(response);
    reset_alarm_count();
    reset_state();

    while (get_alarm_count() < 3 && get_curr_state() != STOP) {
        set_alarm_flag(FALSE);

        if ((bytes = write(fd, frame, msg_size)) == -1) {
            printf("Write failed\n");
            return -1;
        }
        
        printf("Message sent\n");
        unsigned char buf[MSG_MAX_SIZE*2] = {0};
        alarm(3);
        int i = 0;
        
        while (get_curr_state() != STOP && !get_alarm_flag()) {
            if (i >= (MSG_MAX_SIZE*2)) continue;
            
            int read_byte = read(fd, buf + i, 1);
            if (read_byte != -1) update_state(buf[i]); 
            
            i++;
        }
    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }
    return bytes;
}


int read_message(int fd, uint8_t *buf, int buf_size, command response) {
    int i = 0;
    reset_state();
    set_command(response);

    while (get_curr_state() != STOP && !get_alarm_flag()) {
        if (i >= buf_size){
            break;
        }

        int bytes = read(fd, buf + i, 1);

        //printf("received -> %x\n", buf[i]);
        if (bytes != -1) {
            update_state(buf[i]);
        }

        i++;
    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }
    return i;
}


uint8_t generate_bcc2(uint8_t *data, int len) {
    uint8_t bcc2 = data[0];

    for (int i = 1; i < len; ++i)
        bcc2 ^= data[i];

    return bcc2;
}

int msg_stuff(uint8_t *buffer, int start, int msg_size, uint8_t *stuffed_msg) {
    int i = 0;

    // Copy header without stuffing
    for (int j = 0; j < start; ++j, ++i)
        stuffed_msg[i] = buffer[j];
    
    // Stuffing
    for (int j = start; j < msg_size; ++j) {
        if (buffer[j] == FLAG || buffer[j] == ESCAPE) {
            stuffed_msg[i++] = ESCAPE;
            stuffed_msg[i++] = buffer[j] ^ 0x20;
        }

        else{
            stuffed_msg[i++] = buffer[j];
        }
    }
    return i;
}

int msg_destuff(uint8_t *buffer, int start, int msg_size, uint8_t *destuffed_msg) {
    int i = 0;

    for (int j = 0; j < start; ++j, ++i)
        destuffed_msg[i] = buffer[j];

    
    for (int j = start; j < msg_size; j++) {
        if (buffer[j] == ESCAPE) {
            destuffed_msg[i] = buffer[j + 1] ^ 0x20;
            j++;
            i++;
        }

        else {
            destuffed_msg[i] = buffer[j];
            i++;
        }
    }
    return i;
}
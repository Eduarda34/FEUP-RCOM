#include "frame.h"

void init_s_frame(s_frame *frame, uint8_t address, uint8_t control) {
    frame->flag_start = FLAG;
    frame->address = address;
    frame->control = control;
    frame->bcc = BCC(frame->address, frame->control);
    frame->flag_end = FLAG;
}


void init_i_frame(i_frame *frame, const uint8_t *data, int data_len, int packet) {
    frame->flag_start = FLAG;
    frame->address = ADDR_E;
    frame->control = (packet << 6);  // Packet number in control field
    frame->bcc1 = BCC(frame->address, frame->control);

    // Allocate and copy the data
    frame->data_len = data_len;
    frame->data = (uint8_t *)malloc(data_len);
    memcpy(frame->data, data, data_len);

    // Calculate BCC2 for the data
    frame->bcc2 = 0;
    for (int i = 0; i < data_len; i++) {
        frame->bcc2 ^= frame->data[i];
    }

    frame->flag_end = FLAG;

    // Allocate the full buffer (frame + data + BCC)
    frame->buffer_len = data_len + 5;  // flag, addr, control, bcc1, bcc2, flag
    frame->buffer = (uint8_t *)malloc(frame->buffer_len);
}


void fill_i_frame_buffer(i_frame *frame) {
    frame->buffer[0] = frame->flag_start;
    frame->buffer[1] = frame->address;
    frame->buffer[2] = frame->control;
    frame->buffer[3] = frame->bcc1;

    // Copy data into the buffer
    for (int i = 0; i < frame->data_len; i++) {
        frame->buffer[i + 4] = frame->data[i];
    }

    // Add BCC2 and flag
    frame->buffer[frame->data_len + 4] = frame->bcc2;
    frame->buffer[frame->data_len + 5] = frame->flag_end;
}

void free_i_frame(i_frame *frame) {
    free(frame->data);
    free(frame->buffer);
}


#ifndef _FRAME_
#define _FRAME_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BCC(a, b) ((a) ^ (b))
#define ADDR_E 0x03
#define CTRL_S(s) ((s == 0) ? 0 : BIT(6))

#define CTRL_SET 0x03
#define CTRL_DISC 0x0B
#define CTRL_UA 0x07
#define CTRL_RR(r) ((r == 0) ? 0x05 : 0x85)
#define CTRL_REJ(r) ((r == 0) ? 0x01 : 0x81)

#define MSG_MAX_SIZE 1000

#define NO_RESP -1

#define RR_1 0x85  // Define the correct value
#define RR_0 0x05  // Define the correct value

// Define the structure for the S-frame
typedef struct
{
    uint8_t flag_start;
    uint8_t address;
    uint8_t control;
    uint8_t bcc;
    uint8_t flag_end;
} s_frame;


typedef struct
{
    uint8_t flag_start;
    uint8_t address;
    uint8_t control;
    uint8_t bcc1;
    uint8_t *data; // Pointer to data buffer
    int data_len;
    uint8_t bcc2;
    uint8_t flag_end;
    uint8_t *buffer;     // Complete buffer with frame and data
    int buffer_len;      // Total length of the buffer
} i_frame;


// Function to initialize the S-frame
void init_s_frame(s_frame *frame, uint8_t address, uint8_t control)
{
    frame->flag_start = FLAG;
    frame->address = address;
    frame->control = control;
    frame->bcc = BCC(frame->address, frame->control);
    frame->flag_end = FLAG;
}

// Function to initialize the I-frame
void init_i_frame(i_frame *frame, const uint8_t *data, int data_len, int packet)
{
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
    for (int i = 0; i < data_len; i++)
    {
        frame->bcc2 ^= frame->data[i];
    }

    frame->flag_end = FLAG;

    // Allocate the full buffer (frame + data + BCC)
    frame->buffer_len = data_len + 5;  // flag, addr, control, bcc1, bcc2, flag
    frame->buffer = (uint8_t *)malloc(frame->buffer_len);
}

// Function to free the memory allocated for the I-frame
void fill_i_frame_buffer(i_frame *frame)
{
    frame->buffer[0] = frame->flag_start;
    frame->buffer[1] = frame->address;
    frame->buffer[2] = frame->control;
    frame->buffer[3] = frame->bcc1;

    // Copy data into the buffer
    for (int i = 0; i < frame->data_len; i++)
    {
        frame->buffer[i + 4] = frame->data[i];
    }

    // Add BCC2 and flag
    frame->buffer[frame->data_len + 4] = frame->bcc2;
    frame->buffer[frame->data_len + 5] = frame->flag_end;
}

// Function to free the memory allocated for the I-frame
void free_i_frame(i_frame *frame)
{
    free(frame->data);
    free(frame->buffer);
}


#endif // _FRAME_ 
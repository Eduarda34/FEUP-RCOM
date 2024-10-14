#ifndef _FRAME_
#define _FRAME_

#include "state.h"


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
typedef struct {
    uint8_t flag_start;
    uint8_t address;
    uint8_t control;
    uint8_t bcc;
    uint8_t flag_end;
} s_frame;


typedef struct {
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


/**
 * @brief initializes an S-frame with specified address and control information
 * 
 * @param frame pointer to the S-frame structure to be initialized
 * @param address address byte to be set in the frame
 * @param control control byte to be set in the frame
 */
void init_s_frame(s_frame *frame, uint8_t address, uint8_t control);

/**
 * @brief initializes an I-frame with specified data, length, and packet number
 * 
 * @param frame pointer to the I-frame structure to be initialized
 * @param data pointer to the data to be included in the frame
 * @param data_len length of the data to be copied into the frame
 * @param packet packet number, which is shifted and stored in the control field
 */
void init_i_frame(i_frame *frame, const uint8_t *data, int data_len, int packet);

/**
 * @brief fills the buffer of an I-frame with its components
 * @param frame pointer to the I-frame structure whose buffer will be filled
 */
void fill_i_frame_buffer(i_frame *frame);
/**
 * @brief frees the allocated memory for an I-frame
 * @param frame pointer to the I-frame structure whose resources will be freed
 */
void free_i_frame(i_frame *frame);
#endif // _FRAME_ 
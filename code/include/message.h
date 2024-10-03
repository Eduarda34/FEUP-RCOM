#ifndef _MESSAGE_
#define _MESSAGE_

#include <state.h>
#include <frame.h>
#include <alarm.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdint.h>

#define FRAME_SIZE 5

// Send S Frame (Supervisory Frame)
// Parameters:
//
int send_s_frame(int fd, uint8_t address, uint8_t control, command response);

// Send I Frame (Information Frame)
// Parameters:
//
int send_i_frame(int fd, const uint8_t *data, int data_len, int packet);

// Send Message
// Parameters:
//
int send_message(int fd, uint8_t* buffer, int n, command response);

// Read Message 
// Parameters:
//
int read_message(int fd, uint8_t* buffer, int n, command response);

//
// Parameters:
//
int msg_stuff(uint8_t *buffer, int start, int msg_size, uint8_t *stuffed_msg);

//
// Parameters:
//
int msg_destuff(uint8_t *buffer, int start, int msg_size, uint8_t *destuffed_msg);

//
// Parameters:
//
uint8_t generate_bcc2(uint8_t *data, int len);

#endif // _MESSAGE_

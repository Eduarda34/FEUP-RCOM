#ifndef _FRAME_FUNCTIONS_H_
#define _FRAME_FUNCTIONS_H_

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

// Frame Functions header
// Arguments:
// 
//
void construct_frame(uint8_t *buffer, uint8_t address, uint8_t control);


int send_frame(int fd, uint8_t address, uint8_t control, char response);

int send_message(int fd, uint8_t* buffer, int n, char response);

#endif //_FRAME_FUNCTIONS_H_
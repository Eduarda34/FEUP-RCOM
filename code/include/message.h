#pragma once

#ifndef _MESSAGE_
#define _MESSAGE_

#include <state.h>
#include <frame.h>
#include "alarm.h"

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

/**
 * @brief sends an S-frame over a specified file descriptor
 * 
 * @param fd file descriptor where the S-frame will be sent
 * @param address address byte to be used in the S-frame
 * @param control control byte that defines the type of S-frame
 * @param response specifies whether this is a command or a response type
 * @return number of bytes sent, or -1 if an error occurred
 */
int send_s_frame(int fd, uint8_t address, uint8_t control, command response);

/**
 * @brief sends an I-frame with the specified data and packet number over a given file descriptor
 * 
 * @param fd file descriptor to which the I-frame will be sent
 * @param data pointer to the data payload to be included in the I-frame.
 * @param data_len length of the data payload in bytes
 * @param packet packet sequence number (0 or 1), used to track the sequence of frames
 * @return number of bytes sent on success, or -1 if the message could not be sent 
 */
int send_i_frame(int fd, const uint8_t *data, int data_len, int packet);

/**
 * @brief sends a message frame over the specified file descriptor and optionally waits for a response
 * 
 * @param fd file descriptor where the message will be sent
 * @param buffer pointer to the byte array representing the frame to be sent
 * @param n size of the message frame in bytes
 * @param response indicates whether a response is expected
 * @return number of bytes successfully written, or -1 if a failure occurred during transmission
 */
int send_message(int fd, uint8_t* buffer, int n, command response);

/**
 * @brief reads a message from the specified file descriptor and updates the state machine
 * 
 * @param fd file descriptor from which the message is read
 * @param buffer pointer to the buffer where the received message will be stored
 * @param n maximum size of the buffer (number of bytes to read)
 * @param response expected type of response
 * @return number of bytes successfully read 
 */
int read_message(int fd, uint8_t* buffer, int n, command response);

/**
 * @brief performs byte stuffing on a message to escape special flag and escape characters
 * 
 * @param buffer pointer to the original message buffer 
 * @param start index in the buffer where stuffing should begin
 * @param msg_size total size of the original message
 * @param stuffed_msg pointer to the buffer where the stuffed message will be stored
 *
 * @return total size of the stuffed message  
 */
int msg_stuff(uint8_t *buffer, int start, int msg_size, uint8_t *stuffed_msg);

/**
 * @brief removes byte stuffing from a message to restore the original data
 * 
 * @param buffer pointer to the received message buffer that contains stuffed data
 * @param start index in the buffer where destuffing should begin
 * @param msg_size total size of the stuffed message
 * @param destuffed_msg pointer to the buffer where the destuffed (the original) message will be stored
 * @return total size of the destuffed message
 */
int msg_destuff(uint8_t *buffer, int start, int msg_size, uint8_t *destuffed_msg);

/**
 * @brief generates the BCC2 (Block Check Character) for error detection using XOR
 * 
 * @param data pointer to the byte array containing the data over which the BCC2 will be calculated
 * @param len length of the data array
 * @return uint8_t 
 */
uint8_t generate_bcc2(uint8_t *data, int len);

#endif // _MESSAGE_

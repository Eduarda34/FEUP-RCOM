#ifndef _STATE_
#define _STATE_

#include <stdint.h>
#include <stdio.h>

#define FLAG 0x7E
#define ESCAPE 0x7d
#define ADDR 0x03

typedef enum
{
    COMMAND_SET,        // SET COMMAND 
    COMMAND_DISC,       // DISCONNECT COMMAND 
    COMMAND_DATA,       // DATA TRANSFER COMMAND
    RESPONSE_UA,        // UNNUMBERED ACKNOWLEDGMENT RESPONSE
    RESPONSE_REJ        // RECEIVER READY/REJECT RESPONSES
} command;

typedef enum
{
    TRANSMITTER,
    RECEIVER
} role;

typedef enum {
    RESPONSE_NULL,      // INIT STATE
    UA,                 // UNNUMBERED ACKNOWLEDGMENT RECEIVED
    PA_F0,              // POSITIVE ACKNOWLEDGMENT FRAME 0
    PA_F1,              // POSITIVE ACKNOWLEDGMENT FRAME 1
    REJ_F0,             // REJECT FRAME 0
    REJ_F1              // REJECT FRAME 1
} response;

typedef enum {
    START_STATE,        // Init State
    FLAG_RCV,           // Flag Byte Received 
    ADDRESS_RCV,        // Address Byte Received
    CONTROL_RCV,        // Control Byte Received 
    DATA_RCV,           // Data Byte Received
    BCC_VER,            // BCC Verification Passed
    FINAL_STATE         // Final State
} state;

typedef struct  {
    state curr_state;       // Current State
    uint8_t address;        // Address Byte
    uint8_t control;        // Control Byte
    response prev_response; // Previous response
    role curr_role;         // Current Role
    command curr_command;   // Current Comand 
} state_machine;

////////////////////////////////////////////////
// GETTERS
////////////////////////////////////////////////
uint8_t get_address();
uint8_t get_control();

/**
 * @brief Get the current state of the machine
 * @return current state
 */
state get_curr_state();

/**
 * @brief Get the current role object
 * @return role 
 */
role get_curr_role();

/**
 * @brief Get the curr command object
 * @return command currently being handled
 */
command get_curr_command();

/**
 * @brief Get the prev response object
 * @return response 
 */
response get_prev_response();

////////////////////////////////////////////////
// SETTERS
////////////////////////////////////////////////

/**
 * @brief Set the state object
 * @param s 
 */
void set_state(state s);

/**
 * @brief Set the role object
 * @param r 
 */
void set_role(role r);

/**
 * @brief Set the command object
 * @param c 
 */
void set_command(command c);

////////////////////////////////////////////////
// MAIN FUNCTIONS
////////////////////////////////////////////////

/**
 * @brief processes a received byte based on the current state of the state machine, transitioning between different states according to the received protocol frame
 * @param byte 
 */
void update_state(unsigned char byte);

/**
 * @brief resets the state machine to its initial state
 */
void reset_state();

#endif // _STATE_
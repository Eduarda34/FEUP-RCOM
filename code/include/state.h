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
    PA_F0,              // Positive acknowledgment (RR) for frame 0
    PA_F1,              // Positive acknowledgment (RR) for frame 1
    REJ_F0,             // Reject (REJ) for frame 0
    REJ_F1              // Reject (REJ) for frame 1
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
state get_curr_state();
role get_curr_role();
command get_curr_command();
response get_prev_response();

////////////////////////////////////////////////
// SETTERS
////////////////////////////////////////////////
void set_state(state s);
void set_role(role r);
void set_command(command c);

////////////////////////////////////////////////
// MAIN FUNCTIONS
////////////////////////////////////////////////
void update_state(unsigned char byte);
void reset_state();

#endif // _STATE_
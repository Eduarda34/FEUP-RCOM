#ifndef _STATE_
#define _STATE_

#include <stdint.h>
#include <stdio.h>

#define FLAG 0x7E
#define ESCAPE 0x7d
#define ADDR 0x03

#define TRUE 1
#define FALSE 0

#define BIT(n) (1 << n)


typedef enum 
{
    COMMAND_SET,        // Set Command 0x03?? 
    COMMAND_DISC,       // Disconnect Command
    COMMAND_DATA,       // ??
    R_UA,               // ??
    R_RR_REJ,           // ?? 
} command;


typedef enum 
{
    TRANSMITTER,    // Machine Role Transmitter
    RECEIVER        // Machine Role Receiver
} role;


typedef enum 
{
    R_NULL,         // ??
    UA,             // ??
    PR_0,           // ??
    PR_1,           // ??
    REJ_0,          // ??
    REJ_1           // ??
} response;


typedef enum 
{
    START,          // Start State
    FLAG_RCV,       // Flag State Received 
    ADDRESS_RCV,    // Address Received
    CONTROL_RCV,    // Control State Received 
    DATA_RCV,       // Data State Received
    BCC_VER,        // BCC Verification Passed State
    STOP            // Stop State
} state;



typedef struct 
{
    state CURR_STATE;       // Current State
    uint8_t ADDRESS;        // Address Byte
    uint8_t CONTROL;        // Control Byte
    response PREV_RESPONSE; // Previous response
    role CURR_ROLE;         // Current Role
    command CURR_COMMAND;   // Current Comand 
} state_machine;



// Getters
unsigned char get_address();
unsigned char get_control();
state get_curr_state();
role get_curr_role();
command get_curr_command();
response get_prev_response();



// Setters
void set_address(unsigned char s);
void set_control(unsigned char s);
void set_state(state s);
void set_role(role r);
void set_command(command c);
void set_response(response r);


// Others functions
void update_state(unsigned char byte);

void reset_state();





#endif // _STATE_

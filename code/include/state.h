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


typedef enum {
    COMMAND_SET,        // Set Command 
    COMMAND_DISC,       // Disconnect Command
    COMMAND_DATA,       // ??
    R_UA,               // ??
    R_RR_REJ,           // ?? 
} command;


typedef enum {
    TRANSMITTER,    // Machine Role Transmitter
    RECEIVER        // Machine Role Receiver
} role;


typedef enum {
    R_NULL,         // ??
    UA,             // ??
    PR_0,           // ??
    PR_1,           // ??
    REJ_0,          // ??
    REJ_1           // ??
} response;


typedef enum {
    START,          // Start State
    FLAG_RCV,       // Flag State Received 
    ADDRESS_RCV,    // Address Received
    CONTROL_RCV,    // Control State Received 
    DATA_RCV,       // Data State Received
    BCC_VER,        // BCC Verification Passed State
    STOP            // Stop State
} state;



typedef struct  {
    state CURR_STATE;       // Current State
    uint8_t ADDRESS;        // Address Byte
    uint8_t CONTROL;        // Control Byte
    response PREV_RESPONSE; // Previous response
    role CURR_ROLE;         // Current Role
    command CURR_COMMAND;   // Current Comand 
} state_machine;



// Getters
/**
 * @brief Get the address object
 * @return unsigned char 
 */
unsigned char get_address();

/**
 * @brief Get the control object
 * @return unsigned char 
 */
unsigned char get_control();

/**
 * @brief Get the curr state object
 * @return state 
 */
state get_curr_state();

/**
 * @brief Get the curr role object
 * @return role 
 */
role get_curr_role();

/**
 * @brief Get the curr command object
 * @return command 
 */
command get_curr_command();

/**
 * @brief Get the prev response object
 * @return response 
 */
response get_prev_response();


// Setters
/**
 * @brief Set the address object
 * @param s 
 */
void set_address(unsigned char s);

/**
 * @brief Set the control object
 * @param s 
 */
void set_control(unsigned char s);

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

/**
 * @brief Set the response object
 * @param r 
 */
void set_response(response r);


// Others functions
/**
 * @brief updates the current state of the state machine based on the received byte
 * @param byte - to be processed by the state machine
 */
void update_state(unsigned char byte);

/**
 * @brief resets the state machine to its initial state
 */
void reset_state();

#endif // _STATE_
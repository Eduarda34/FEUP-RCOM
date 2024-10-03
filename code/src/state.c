#include "state.h"

state_machine state_m;

// Getters 
unsigned char get_address() {
    return state_m.ADDRESS;
}

unsigned char get_control() {
    return state_m.CONTROL;
}

state get_curr_state() {
    return state_m.CURR_STATE;
}

role get_curr_role() {
    return state_m.CURR_ROLE;
}

command get_curr_command() {
    return state_m.CURR_COMMAND;
}

response get_prev_response() {
    return state_m.PREV_RESPONSE;
}


// Setters
void set_address(unsigned char s) {
    state_m.ADDRESS = s;
}

void set_control(unsigned char c) {
    state_m.CONTROL = c;
}

void set_state(state s) {
    state_m.CURR_STATE = s;
}

void set_role(role r) {
    state_m.CURR_ROLE = r;
}

void set_command(command c) {
    state_m.CURR_COMMAND = c;
}

void set_response(response r) {
    state_m.PREV_RESPONSE = r;
}


// Others fucntions
void update_state(unsigned char byte) {
    switch (state_m.CURR_STATE)
    {
    case START:
        if (byte == FLAG)
            set_state(FLAG_RCV);
        break;
    case FLAG_RCV:
        if (byte == FLAG)
            break;
        else if (byte == ADDR)
        {
            set_address(byte);
            set_state(ADDRESS_RCV);
        }
        else
        {
            set_state(START);
        }

        break;
    case ADDRESS_RCV:
        if (byte == FLAG)
            set_state(FLAG_RCV);
        else if (byte == 0x03 && get_curr_command() == COMMAND_SET)
        {
            set_control(byte);
            set_state(CONTROL_RCV);
        }
        else if (byte == 0x07 && get_curr_command() == R_UA)
        {
            set_control(byte);
            set_state(CONTROL_RCV);
        }
        else if (byte == 0x0B && get_curr_command() == COMMAND_DISC)
        {
            set_control(byte);
            set_state(CONTROL_RCV);
        }
        else if ((byte == 0x00 || byte == 0x40) && get_curr_command() == COMMAND_DATA)
        {
            set_control(byte);
            set_state(CONTROL_RCV);
        }
        else if ((byte == 0x05 || byte == 0x85 || byte == 0x01 || byte == 0x81) && get_curr_command() == R_RR_REJ)
        {
            set_control(byte);
            set_state(CONTROL_RCV);
            switch (byte)
            {
            case 0x05:
                set_response(PR_0);
                break;
            case 0x85:
                set_response(PR_1);
                break;
            case 0x01:
                set_response(REJ_0);
                break;
            case 0x81:
                set_response(REJ_1);
                break;
            }
        }
        else
        {
            set_state(START);
        }
        break;
    case CONTROL_RCV:
        if (byte == (state_m.ADDRESS ^ state_m.CONTROL))
        {
            if (get_curr_command() == COMMAND_DATA)
                set_state(DATA_RCV);
            else
                set_state(BCC_VER);
        }
        else if (byte == FLAG)
            set_state(FLAG_RCV);
        else
            set_state(START);
        break;
    case BCC_VER:
        if (byte == FLAG)
            set_state(STOP);
        else
            set_state(START);
        break;
    case DATA_RCV
    :
        if (byte == FLAG)
            set_state(STOP);
        break;
    case STOP:
        break;
    }
}

void reset_state()
{
    state_m.CURR_STATE = START;
    state_m.PREV_RESPONSE = R_NULL;
}
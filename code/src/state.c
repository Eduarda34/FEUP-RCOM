#include "state.h"


state_machine state_m;

////////////////////////////////////////////////
// GETTERS
////////////////////////////////////////////////
unsigned char get_address() {
    return state_m.address;
}

unsigned char get_control() {
    return state_m.control;
}

state get_curr_state() {
    return state_m.curr_state;
}

role get_curr_role() {
    return state_m.curr_role;
}

command get_curr_command() {
    return state_m.curr_command;
}

response get_prev_response() {
    return state_m.prev_response;
}

////////////////////////////////////////////////
// SETTERS
////////////////////////////////////////////////
void set_address(unsigned char s) {
    state_m.address = s;
}

void set_control(unsigned char c) {
    state_m.control = c;
}

void set_state(state s) {
    state_m.curr_state = s;
}

void set_role(role r) {
    state_m.curr_role = r;
}

void set_command(command c) {
    state_m.curr_command = c;
}

void set_response(response r) {
    state_m.prev_response = r;
}


////////////////////////////////////////////////
// MAIN FUNCTIONS
////////////////////////////////////////////////
void update_state(unsigned char byte)
{
    switch (state_m.curr_state)
    {

    case START_STATE:
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
            set_state(START_STATE);
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
        else if (byte == 0x07 && get_curr_command() == RESPONSE_UA)
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
        else if ((byte == 0x05 || byte == 0x85 || byte == 0x01 || byte == 0x81) && get_curr_command() == RESPONSE_REJ)
        {
            set_control(byte);
            set_state(CONTROL_RCV);
            switch (byte)
            {
            case 0x05:
                set_response(PA_F0);
                break;
            case 0x85:
                set_response(PA_F1);
                break;
            case 0x01:
                set_response(REJ_F0);
                break;
            case 0x81:
                set_response(REJ_F1);
                break;
            }
        }
        else
        {
            set_state(START_STATE);
        }
        break;

    case CONTROL_RCV:
        if (byte == (state_m.address ^ state_m.control))
        {
            if (get_curr_command() == COMMAND_DATA)
                set_state(BCC_VER);
            else
                set_state(DATA_RCV);
        }
        else if (byte == FLAG)
            set_state(FLAG_RCV);
        else
            set_state(START_STATE);
        break;
        
    case DATA_RCV:
        if (byte == FLAG)
            set_state(FINAL_STATE);
        else
            set_state(START_STATE);
        break;

    case BCC_VER:
        if (byte == FLAG)
            set_state(FINAL_STATE);
        break;

    case FINAL_STATE:
        break;
    }
}

void reset_state()
{
    state_m.curr_state = START_STATE;
    state_m.prev_response = RESPONSE_NULL;
}
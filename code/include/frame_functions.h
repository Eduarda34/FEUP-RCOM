#ifndef _FRAME_FUNCTIONS_H_
#define _FRAME_FUNCTIONS_H_

// Frame Functions header
// Arguments:
// 
//
void construct_frame(uint8_t *buffer, uint8_t address, uint8_t control);


int send_frame(int fd, uint8_t address, uint8_t control, command response);



#endif //_FRAME_FUNCTIONS_H_
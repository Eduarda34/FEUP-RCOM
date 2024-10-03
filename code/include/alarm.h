// Alarm functions header.

#ifndef _ALARM_H_
#define _ALARM_H_

#define TRUE 1
#define FALSE 0

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

// Main Alarm Functions 
void reset_alarm_count();
void alarm_handler(int signal);

// Getters
int get_alarm_count();
int get_alarm_flag();

// Setters
void set_alarm_flag(int flag);



#endif // _ALARM_H_

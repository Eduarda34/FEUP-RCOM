// Alarm functions header.

#ifndef _ALARM_H_
#define _ALARM_H_

#define TRUE 1
#define FALSE 0

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

// Main Alarm Functions
/**
 * @brief resets the alarm counter to zero
 */
void reset_alarm_count();

/**
 * @brief 
 * @param signal 
 */
void alarm_handler(int signal);

// Getters
/**
 * @brief Get the alarm count object
 * @return int 
 */
int get_alarm_count();

/**
 * @brief Get the alarm flag object
 * @return int 
 */
int get_alarm_flag();

// Setters
/**
 * @brief Set the alarm flag object
 * @param flag 
 */
void set_alarm_flag(int flag);



#endif // _ALARM_H_
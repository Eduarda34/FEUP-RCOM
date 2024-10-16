#include "alarm.h"  

int alarmEnabled = FALSE;
int alarmCount = 0;

void reset_alarm_count() {
    alarmCount = 0; 
}

void alarm_handler(int signal)  
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}

int get_alarm_count() {
    return alarmCount;
}

int get_alarm_flag() {
    return alarmEnabled; 
}

void set_alarm_flag(int flag) {
    alarmEnabled = flag; 
}

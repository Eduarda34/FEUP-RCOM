// Alarm functions implementation

#include <alarm.h>

int alarm_flag = FALSE;
int alarm_count = 0;

void reset_alarm_count(){
    alarm_count = 0;
}

void alarm_handler(int signal) {
    set_alarm_flag(TRUE);
    alarm_count++;

    printf("Alarm count: #%d\n", alarm_count);
}

int get_alarm_count() {
    return alarm_count;
}

int get_alarm_flag() {
    return alarm_flag;
}

void set_alarm_flag(int flag) {
    alarm_flag = flag;
}

#ifndef ROBOCOOP_H
#define ROBOCOOP_H


typedef enum machine_state {
  READY,
  SET,
  OPENING,
  CLOSING,
  MANUAL,
  ERROR_STATE
} machine_state_t;

typedef enum set_time_state {
  OPEN_HOUR,
  OPEN_MINUTE,
  CLOSE_HOUR,
  CLOSE_MINUTE,
  TIME_HOUR,
  TIME_MINUTE
} set_time_state_t;

typedef struct alarm {
  int hour;
  int minute;
} alarm_s;

#endif

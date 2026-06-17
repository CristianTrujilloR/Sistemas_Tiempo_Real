#ifndef SERVO_MOTOR_H_
#define SERVO_MOTOR_H_

#include <stdint.h>

void servo_init(void);

void servo_set_percent(uint8_t percent);

uint8_t servo_get_percent(void);

#endif
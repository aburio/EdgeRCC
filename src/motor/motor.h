#ifndef __MOTOR_INCLUDED__
#define __MOTOR_INCLUDED__

#include "bsp.h"

void motor_init(const uint8_t pwn_pin, const uint8_t input1_pin, const uint8_t input2_pin);
void motor_task(void *pvParameters);

#endif
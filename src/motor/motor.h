#ifndef __MOTOR_INCLUDED__
#define __MOTOR_INCLUDED__

#include "bsp.h"

void motorInit(const uint8_t pwn_pin, const uint16_t pwm_frequency, const uint8_t pwm_resolution, const uint8_t input1_pin, const uint8_t input2_pin);
void motorTask(void *pvParameters);

bool motorSetDutyCycle(uint32_t value);
uint32_t motorGetDutyCycle();

bool motorSetDirection(int8_t direction);

#endif
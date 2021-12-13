#include "motor.h"

/* local types & variables */
uint8_t motor_pwm_resolution = 0;
uint16_t motor_pwm_frequency = 0;
uint32_t motor_pwm_setpoint = 0;

/* private functions prototypes */

/* public functions */
void motorInit(const uint8_t pwn_pin, const uint16_t pwm_frequency, const uint8_t pwm_resolution, const uint8_t input1_pin, const uint8_t input2_pin)
{
    motor_pwm_frequency = pwm_frequency;
    motor_pwm_resolution = pwm_resolution;

    pinMode(input1_pin, OUTPUT);
    digitalWrite(input1_pin, LOW);
    pinMode(input2_pin, OUTPUT);
    digitalWrite(input1_pin, LOW);
    ledcSetup(0, motor_pwm_frequency, motor_pwm_resolution);
    ledcAttachPin(pwn_pin ,0);
}

bool motorSetDutyCycle(uint32_t value)
{
    if (value > (pow(2, motor_pwm_resolution) - 1))
    {
        return false;
    }
    else
    {
        motor_pwm_setpoint = value;
    }

    return true;
}

uint32_t motorGetDutyCycle()
{
    return ledcRead(0);
}

bool motorSetDirection(int8_t direction)
{
    if (direction == -1)
    {
        digitalWrite(INPUT1_PIN, LOW);
        digitalWrite(INPUT2_PIN, HIGH);
    }
    else if (direction == 0)
    {
        digitalWrite(INPUT1_PIN, LOW);
        digitalWrite(INPUT2_PIN, LOW);
        motorSetDutyCycle(0);
    }
    else if (direction == 1)
    {
        digitalWrite(INPUT1_PIN, HIGH);
        digitalWrite(INPUT2_PIN, LOW);
    }
    else {
        return false;
    }

    return true;
}

void motorTask(void *pvParameters)
{
    for (;;)
    {

        if (motor_pwm_setpoint > motorGetDutyCycle())
        {
            uint32_t value = motorGetDutyCycle() + 2;

            if (value > motor_pwm_setpoint)
            {
                ledcWrite(0, motor_pwm_setpoint);
            }
            else
            {
                ledcWrite(0, value);
            }
        }
        else if (motor_pwm_setpoint < motorGetDutyCycle())
        {
            uint32_t value = motorGetDutyCycle() - 2;

            if (value < motor_pwm_setpoint)
            {
                ledcWrite(0, motor_pwm_setpoint);
            }
            else
            {
                ledcWrite(0, value);
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
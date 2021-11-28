#include "motor.h"

/* local types & variables */

/* private functions prototypes */

/* public functions */
void motor_init(const uint8_t pwn_pin, const uint8_t input1_pin, const uint8_t input2_pin)
{
    pinMode(input1_pin, OUTPUT);
    digitalWrite(input1_pin, LOW);
    pinMode(input2_pin, OUTPUT);
    digitalWrite(input1_pin, LOW);
    ledcSetup(0, 50, 16);
    ledcAttachPin(pwn_pin ,0);
}

void motor_task(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/* private functions */
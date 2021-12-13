#include "function.h"

/* local types & variables */

/* private functions prototypes */

/* public functions */
void functionInit()
{
    pinMode(FRONT_CAB, OUTPUT_OPEN_DRAIN);
    digitalWrite(FRONT_CAB, 1);
    pinMode(FRONT_WHITE, OUTPUT_OPEN_DRAIN);
    digitalWrite(FRONT_WHITE, 1);
    pinMode(FRONT_RED, OUTPUT_OPEN_DRAIN);
    digitalWrite(FRONT_RED, 1);
    pinMode(REAR_CAB, OUTPUT_OPEN_DRAIN);
    digitalWrite(REAR_CAB, 1);
    pinMode(REAR_WHITE, OUTPUT_OPEN_DRAIN);
    digitalWrite(REAR_WHITE, 1);
    pinMode(REAR_RED, OUTPUT_OPEN_DRAIN);
    digitalWrite(REAR_RED, 1);
}

void functionTask(void *pvParameters)
{
    vTaskDelete(NULL);
}

/* private functions */
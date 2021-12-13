#include "controller.h"

#include <ArduinoJson.h>

#include "motor/motor.h"

/* local types & variables */
struct controllermessage ctrl_message;
QueueHandle_t controllerqueue = NULL;

/* private functions prototypes */

/* public functions */
void controllerInit()
{
    controllerqueue = xQueueCreate( 5, sizeof( struct controllermessage ) );
}

void controllerTask(void *pvParameters)
{
    for (;;)
    {
        if (xQueueReceive(controllerqueue, &ctrl_message, portMAX_DELAY) == pdPASS)
        {
            log_v("function : %s", ctrl_message.function_name);
            log_v("value : %u", ctrl_message.function_value);

            if (ctrl_message.function_name == "traction")
            {
                motorSetDutyCycle(ctrl_message.function_value);
            }
            else if (ctrl_message.function_name == "direction")
            {
                motorSetDirection(ctrl_message.function_value);
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/* private functions */
#ifndef __CONTROLLER_INCLUDED__
#define __CONTROLLER_INCLUDED__

#include "bsp.h"

struct controllermessage {
    String function_name;
    int32_t function_value;
};

void controllerInit();
void controllerTask(void *pvParameters);

#endif
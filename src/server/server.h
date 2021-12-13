#ifndef __SERVER_INCLUDED__
#define __SERVER_INCLUDED__

#include "bsp.h"

void serverInit(const String local_hostname, bool OTA);
void serverTask(void *pvParameters);

#endif
#ifndef __SERVER_INCLUDED__
#define __SERVER_INCLUDED__

#include "bsp.h"

void server_init(const String local_hostname, bool OTA);
void server_task(void *pvParameters);

#endif
#ifndef __WIFI_INCLUDED__
#define __WIFI_INCLUDED__

#include "bsp.h"

void wifi_init(const String ssid, const String password);
void wifi_task(void *pvParameters);

#endif
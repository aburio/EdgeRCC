#ifndef __WIFI_INCLUDED__
#define __WIFI_INCLUDED__

#include "bsp.h"

void wifiInit(const String ssid, const String password);
void wifiTask(void *pvParameters);

#endif
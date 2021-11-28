#include "wifi.h"

#include <WiFi.h>

/* local types & variables */

/* private functions prototypes */

/* public functions */
void wifi_init(const String ssid, const String password)
{
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("Open-Loco");
    WiFi.setAutoConnect(false);
    WiFi.begin(ssid.c_str(), password.c_str());
    log_v("wifi_init : init done");
}

void wifi_task(void *pvParameters)
{
    bool wifi_connected = false;
    wl_status_t wifi_status = WiFi.status();

    log_v("wifi_task : start task");

    if (WiFi.status() == WL_IDLE_STATUS)
    {
        WiFi.begin();
    }

    for(;;)
    {
        wifi_status = WiFi.status();

        switch (wifi_status)
        {
        case WL_CONNECTED:
            if (wifi_connected == false)
            {
                log_v("wifi_task : connected");
                wifi_connected = true;
            }
            break;
        
        case WL_NO_SSID_AVAIL:
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
            log_v("wifi_task : reconnecting");
            WiFi.reconnect();
            break;
        
        case WL_IDLE_STATUS:
        case WL_DISCONNECTED:
        default:
            log_v("wifi_task : connecting");
            vTaskDelay(100/portTICK_PERIOD_MS);
            break;
        }
    }
    
    vTaskDelete(NULL);
}

/* private functions */
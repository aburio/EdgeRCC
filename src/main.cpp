#include "bsp.h"
#include "user_wifi.h"

#include "function/function.h"
#include "motor/motor.h"
#include "server/server.h"
#include "wifi/wifi.h"

#if !defined(SSID) || !defined(PASSWORD)
#error "Create a user_wifi.h header and add SSID & PASSWORD define to allow wifi connection"
#endif

/**
 * Initialization
 */
void setup(){
  // logger
  if (CORE_DEBUG_LEVEL == 6)
  {
    Serial.begin(115200);
  }

  // init
  function_init();
  motor_init(PWM_PIN, INPUT1_PIN, INPUT2_PIN);
  wifi_init(SSID, PASSWORD);
  server_init("cc6505", true);

  // task core 1
  xTaskCreatePinnedToCore(wifi_task, "WifiTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(server_task, "ServerTask", 32768, NULL, 1, NULL, 1);
}

/**
 * Processing loop
 */
void loop() {}
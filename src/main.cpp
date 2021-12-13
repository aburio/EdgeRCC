#include "bsp.h"
#include "user_wifi.h"

#include "controller/controller.h"
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
  controllerInit();
  functionInit();
  motorInit(PWM_PIN, 150, 7, INPUT1_PIN, INPUT2_PIN);
  wifiInit(SSID, PASSWORD);
  serverInit("cc6505", true);

  // task core 0
  xTaskCreatePinnedToCore(controllerTask, "controllerTask", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(serverTask, "serverTask", 32768, NULL, 1, NULL, 0);

  // task core 1
  xTaskCreatePinnedToCore(motorTask, "motorTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 2048, NULL, 1, NULL, 1);
}

/**
 * Processing loop
 */
void loop() {}
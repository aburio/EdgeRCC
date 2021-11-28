# EdgeRCC

EdgeRCC is an open-source firmware for remote command control of model trains over Wifi.

## Hardware requirements

- ESP32 - Wifi/BLE SoC
- H-bridge circuit

## Software dependencies

- IDE - [Platformio](https://platformio.org/)
- Core - [arduino-esp32](https://github.com/espressif/arduino-esp32)

## Settings

In order to compile this project you have to create a `user_wifi.h` header in the `include` folder

```
#ifndef __USER_WIFI_INCLUDED__
#define __USER_WIFI_INCLUDED__

#define SSID        "xxx"
#define PASSWORD    "yyy"

#endif
```

replace `xxx` with your Wifi SSID and `yyy` with your Wifi password
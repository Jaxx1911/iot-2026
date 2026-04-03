#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "sensors.h"

#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C

class OledDisplay {
public:
    bool begin();
    void showSplash();
    void update(const BmsReadings& r, bool mqttOk, bool wifiOk,
                bool chargeOn, bool dischargeOn);
    void showAlert(const char* alertType, int cellIndex, float value);

private:
    Adafruit_SSD1306 _oled{OLED_WIDTH, OLED_HEIGHT, &Wire, -1};
    unsigned long _lastPageSwitch = 0;
    uint8_t _page = 0;

    void _drawPage0(const BmsReadings& r, bool mqttOk, bool wifiOk);
    void _drawPage1(const BmsReadings& r, bool chargeOn, bool dischargeOn);
};

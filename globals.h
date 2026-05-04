#pragma once

#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

extern LiquidCrystal_I2C lcd;
extern WiFiClientSecure wifiClient;
extern PubSubClient mqttClient;
extern Adafruit_INA219 ina219;
extern OneWire oneWire;
extern DallasTemperature sensors;

extern float g_percentDisp;
extern float g_tempDisp;
extern int g_modeLastSample;
extern int g_modeSameCount;
extern int g_modeDisplay;

extern bool g_autoThermalCut;
extern RelayMode g_relayMode;
extern bool g_relayCutApplied;

extern float g_lastVBus;
extern float g_lastVAvg;
extern float g_lastCurrent;
extern float g_lastTempC;
extern float g_lastPercent;
extern bool g_lastCharging;
extern bool g_lastTempValid;

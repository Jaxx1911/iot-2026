#pragma once
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_INA219.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "config.h"

struct BmsReadings {
    float cellVoltages[NUM_CELLS];
    float packVoltage;
    float current;
    float temperatures[NUM_TEMP_SENSORS];
    float soc;
    bool valid;
};

class BmsSensors {
public:
    bool begin();
    BmsReadings read();

private:
    Adafruit_ADS1115 _ads;
    Adafruit_INA219  _ina;
    OneWire          _oneWire{ONE_WIRE_PIN};
    DallasTemperature _tempSensors{&_oneWire};

    float _readCellVoltage(uint8_t channel);
    float _estimateSOC(float avgCellV);
};

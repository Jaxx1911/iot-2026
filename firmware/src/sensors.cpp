#include "sensors.h"

bool BmsSensors::begin() {
    if (!_ads.begin()) {
        Serial.println("[SENSOR] ADS1115 not found");
        return false;
    }
    _ads.setGain(GAIN_ONE); // ±4.096V range

    if (!_ina.begin()) {
        Serial.println("[SENSOR] INA219 not found");
        return false;
    }

    _tempSensors.begin();
    Serial.printf("[SENSOR] Found %d DS18B20 sensor(s)\n",
                  _tempSensors.getDeviceCount());
    return true;
}

float BmsSensors::_readCellVoltage(uint8_t channel) {
    int16_t raw = _ads.readADC_SingleEnded(channel);
    // ADS1115 at GAIN_ONE: 1 bit = 0.125 mV
    float adcVoltage = raw * 0.000125f;
    return adcVoltage * VDIV_RATIO;
}

float BmsSensors::_estimateSOC(float avgCellV) {
    // Simple linear SOC estimation for LiFePO4
    // 2.5V = 0%, 3.65V = 100%
    float soc = (avgCellV - CELL_UV_THRESHOLD)
              / (CELL_OV_THRESHOLD - CELL_UV_THRESHOLD) * 100.0f;
    return constrain(soc, 0.0f, 100.0f);
}

BmsReadings BmsSensors::read() {
    BmsReadings r;
    r.valid = true;

    float sum = 0;
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        r.cellVoltages[i] = _readCellVoltage(i);
        sum += r.cellVoltages[i];
    }
    r.packVoltage = sum;

    r.current = _ina.getCurrent_mA() / 1000.0f; // convert to A

    _tempSensors.requestTemperatures();
    for (uint8_t i = 0; i < NUM_TEMP_SENSORS; i++) {
        float t = _tempSensors.getTempCByIndex(i);
        r.temperatures[i] = (t == DEVICE_DISCONNECTED_C) ? -127.0f : t;
    }

    float avg = sum / NUM_CELLS;
    r.soc = _estimateSOC(avg);

    return r;
}

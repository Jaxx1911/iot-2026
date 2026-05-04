#pragma once

#include <Arduino.h>

void batteryMovingAveragePushIf(float vAdj, bool allowPush);
float batteryMovingAverageAvg(void);
float batteryLookupPercent(float v);
void batteryProcessPercentSlew(float percentTarget, uint32_t loopMs);
void batteryProcessTempEma(float tempC);

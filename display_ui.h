#pragma once

#include <Arduino.h>

void ui_lcdPrintFloatOneDecimal(float x);
const char *ui_modeLabel(int m);
int ui_modeFromCurrent(float raw_mA);
void ui_render(float percent, float current, float tempDisp, int modeDisplay, bool stopChargeWarn,
               bool tempValid);

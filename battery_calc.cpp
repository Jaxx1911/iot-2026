#include "battery_calc.h"
#include "config.h"
#include "globals.h"
#include <math.h>

namespace {

struct VoltagePoint {
  float voltage;
  float percentage;
};

const VoltagePoint kBatteryTable[] = {
    {3.00f, 0.0f},  {3.30f, 5.0f},  {3.50f, 12.0f},  {3.60f, 28.0f},
    {3.70f, 50.0f}, {3.75f, 58.0f}, {3.80f, 65.0f}, {3.85f, 72.0f},
    {3.90f, 78.0f}, {3.95f, 85.0f}, {4.00f, 90.0f}, {4.10f, 96.0f},
    {BATTERY_FULL_V, 100.0f},
};
const int kBatteryTableN = sizeof(kBatteryTable) / sizeof(kBatteryTable[0]);

float g_maBuf[MA_VOLTAGE_SAMPLES];
int g_maIdx = 0;
bool g_maPrimed = false;

}  // namespace

void batteryMovingAveragePushIf(float vAdj, bool allowPush) {
  if (!allowPush) {
    return;
  }
  if (!g_maPrimed) {
    for (int i = 0; i < MA_VOLTAGE_SAMPLES; i++) {
      g_maBuf[i] = vAdj;
    }
    g_maPrimed = true;
    g_maIdx = 0;
    return;
  }
  g_maBuf[g_maIdx] = vAdj;
  g_maIdx = (g_maIdx + 1) % MA_VOLTAGE_SAMPLES;
}

float batteryMovingAverageAvg() {
  float s = 0.0f;
  for (int i = 0; i < MA_VOLTAGE_SAMPLES; i++) {
    s += g_maBuf[i];
  }
  return s / (float)MA_VOLTAGE_SAMPLES;
}

float batteryLookupPercent(float v) {
  if (v <= kBatteryTable[0].voltage) {
    return 0.0f;
  }
  if (v >= kBatteryTable[kBatteryTableN - 1].voltage) {
    return 100.0f;
  }
  for (int i = 0; i < kBatteryTableN - 1; i++) {
    float v1 = kBatteryTable[i].voltage;
    float v2 = kBatteryTable[i + 1].voltage;
    if (v >= v1 && v <= v2) {
      float p1 = kBatteryTable[i].percentage;
      float p2 = kBatteryTable[i + 1].percentage;
      return p1 + (v - v1) * (p2 - p1) / (v2 - v1);
    }
  }
  return 0.0f;
}

void batteryProcessPercentSlew(float percentTarget, uint32_t loopMs) {
  if (g_percentDisp < 0.0f) {
    g_percentDisp = percentTarget;
  } else if (PERCENT_MAX_SLEW_PER_S > 0.0f) {
    float maxStep = PERCENT_MAX_SLEW_PER_S * (loopMs / 1000.0f);
    float d = percentTarget - g_percentDisp;
    if (d > maxStep) {
      g_percentDisp += maxStep;
    } else if (d < -maxStep) {
      g_percentDisp -= maxStep;
    } else {
      g_percentDisp = percentTarget;
    }
  } else {
    g_percentDisp += 0.18f * (percentTarget - g_percentDisp);
  }
}

void batteryProcessTempEma(float tempC) {
  if (g_tempDisp < 0.0f) {
    g_tempDisp = tempC;
  } else {
    g_tempDisp += TEMP_SMOOTH_ALPHA * (tempC - g_tempDisp);
  }
}

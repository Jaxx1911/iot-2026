#include <Wire.h>
#include <math.h>
#include "config.h"
#include "globals.h"
#include "battery_calc.h"
#include "display_ui.h"
#include "relay_mqtt.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_INA219 ina219;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float g_percentDisp = -1.0f;
float g_tempDisp = -1.0f;
int g_modeLastSample = -999;
int g_modeSameCount = 0;
int g_modeDisplay = 0;

bool g_autoThermalCut = false;
RelayMode g_relayMode = RELAY_MODE_AUTO;
bool g_relayCutApplied = false;

float g_lastVBus = 0.0f;
float g_lastVAvg = 0.0f;
float g_lastCurrent = 0.0f;
float g_lastTempC = 0.0f;
float g_lastPercent = 0.0f;
bool g_lastCharging = false;
bool g_lastTempValid = false;

void setup() {
  relayInitPinsEarly();
  Serial.begin(115200);
  relayApplyCut(false);

  Wire.begin(21, 22);

  if (!ina219.begin()) {
    Serial.println("INA219 not found!");
    while (1) {
      delay(500);
    }
  }
  ina219.setCalibration_16V_400mA();

  sensors.begin();
  Serial.print("DS18B20 devices: ");
  Serial.println(sensors.getDeviceCount());
  if (sensors.getDeviceCount() == 0) {
    Serial.println("DS18B20: none (check GPIO4, GND, 3V3, 4k7 pull-up DQ->3V3)");
  }

  lcd.init();
  lcd.backlight();

  setupWifi();
  setupMqtt();
}

void loop() {
  const uint32_t loopMs = 1000;

  float raw_mA = ina219.getCurrent_mA();
  float current = raw_mA * INA219_CURRENT_SIGN;
  float vBus = ina219.getBusVoltage_V();

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  const float vCell = vBus;
  const bool charging = (current > CHARGE_I_MA_MIN);
  const bool applyChargeVoltageOffset = charging && (vCell > CHARGE_BU_APPLY_ABOVE_V);
  float vAdj = vCell;
  if (applyChargeVoltageOffset) {
    vAdj -= CHARGE_V_OFFSET;
  }
  const float abs_mA = fabsf(current);
  const bool heavyLoad = (abs_mA >= HEAVY_LOAD_I_MA);
  const bool vbusImplausible = (vBus < VBUS_PLAUSIBLE_LOW_V);
  batteryMovingAveragePushIf(vAdj, !heavyLoad && !vbusImplausible);
  const float avgV = batteryMovingAverageAvg();
  float percentTarget = batteryLookupPercent(avgV);
  percentTarget = constrain(percentTarget, 0.0f, 100.0f);

  const bool tempValid = (tempC > -55.0f && tempC < 125.0f);
  relayThermalUpdate(tempC, tempValid);
  const bool stopChargeWarn = relayUpdateOutput();

  batteryProcessPercentSlew(percentTarget, loopMs);
  if (tempValid) {
    batteryProcessTempEma(tempC);
  }

  int modeNow = ui_modeFromCurrent(raw_mA);
  if (modeNow == g_modeLastSample) {
    if (g_modeSameCount < 255) {
      g_modeSameCount++;
    }
  } else {
    g_modeLastSample = modeNow;
    g_modeSameCount = 1;
  }
  if (g_modeSameCount >= MODE_STABLE_LOOPS) {
    g_modeDisplay = modeNow;
  }

  g_lastVBus = vBus;
  g_lastVAvg = avgV;
  g_lastCurrent = current;
  g_lastTempC = tempC;
  g_lastPercent = g_percentDisp;
  g_lastCharging = charging;
  g_lastTempValid = tempValid;

  Serial.print("Vbus=");
  Serial.print(vBus, 3);
  Serial.print("V Vavg=");
  Serial.print(avgV, 3);
  Serial.print("V");
  if (heavyLoad) {
    Serial.print(" [tải nặng: giữ MA/%]");
  }
  if (vbusImplausible) {
    Serial.print(" [!Vbus<2.5: GND/VIN?]");
  }
  if (applyChargeVoltageOffset) {
    Serial.print(" [bù sạc -");
    Serial.print(CHARGE_V_OFFSET, 2);
    Serial.print("V]");
  } else if (charging) {
    Serial.print(" [sạc V<3.5 k bù]");
  }
  Serial.print(" | I=");
  Serial.print(current, 1);
  Serial.print("mA T=");
  if (tempValid) {
    Serial.print(tempC, 1);
  } else {
    Serial.print("---");
  }
  Serial.print("C P%=");
  Serial.print(g_percentDisp, 1);
  if (stopChargeWarn) {
    Serial.print(" !!RELAY CUT TEMP!!");
  }
  Serial.print(" Relay=");
  Serial.print(g_relayCutApplied ? "CUT" : "CONNECT");
  Serial.print(" Mode=");
  Serial.print(relayModeStr(g_relayMode));
  Serial.println();

  ensureWifi();
  ensureMqtt();
  mqttClient.loop();
  publishTelemetry();

  ui_render(g_percentDisp, current, g_tempDisp, g_modeDisplay, stopChargeWarn, tempValid);

  delay(loopMs);
}

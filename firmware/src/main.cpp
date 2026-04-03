#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors.h"
#include "mqtt_handler.h"
#include "relay_control.h"
#include "oled_display.h"

BmsSensors   sensors;
MqttHandler  mqtt;
RelayControl relays;
OledDisplay  oled;

BmsReadings latestReadings;
unsigned long lastSensorRead  = 0;
unsigned long lastMqttPublish = 0;
unsigned long lastOledUpdate  = 0;

// ── Local protection: runs even without backend connectivity ──
void checkLocalProtection(const BmsReadings& r) {
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        if (r.cellVoltages[i] > CELL_OV_THRESHOLD) {
            Serial.printf("[PROTECT] Cell %d over-voltage: %.3fV\n", i, r.cellVoltages[i]);
            relays.chargeOff();
            oled.showAlert("OVER VOLTAGE", i, r.cellVoltages[i]);
            mqtt.publishAlert("over_voltage", "critical", i,
                              r.cellVoltages[i], CELL_OV_THRESHOLD, "charge_off");
            return;
        }
        if (r.cellVoltages[i] < CELL_UV_THRESHOLD) {
            Serial.printf("[PROTECT] Cell %d under-voltage: %.3fV\n", i, r.cellVoltages[i]);
            relays.dischargeOff();
            oled.showAlert("UNDER VOLTAGE", i, r.cellVoltages[i]);
            mqtt.publishAlert("under_voltage", "critical", i,
                              r.cellVoltages[i], CELL_UV_THRESHOLD, "discharge_off");
            return;
        }
    }

    if (abs(r.current) > PACK_OC_THRESHOLD) {
        Serial.printf("[PROTECT] Over-current: %.3fA\n", r.current);
        relays.allOff();
        oled.showAlert("OVER CURRENT", -1, r.current);
        mqtt.publishAlert("over_current", "critical", -1,
                          r.current, PACK_OC_THRESHOLD, "all_off");
        return;
    }

    for (uint8_t i = 0; i < NUM_TEMP_SENSORS; i++) {
        if (r.temperatures[i] > TEMP_OT_THRESHOLD && r.temperatures[i] != -127.0f) {
            Serial.printf("[PROTECT] Over-temp sensor %d: %.1f°C\n", i, r.temperatures[i]);
            relays.allOff();
            oled.showAlert("OVER TEMP", i, r.temperatures[i]);
            mqtt.publishAlert("over_temperature", "critical", i,
                              r.temperatures[i], TEMP_OT_THRESHOLD, "all_off");
            return;
        }
    }
}

// ── Handle commands from backend ──
void onMqttCommand(const char* cmd, const char* reason) {
    if (strcmp(cmd, "relay_off") == 0) {
        relays.allOff();
    } else if (strcmp(cmd, "charge_off") == 0) {
        relays.chargeOff();
    } else if (strcmp(cmd, "discharge_off") == 0) {
        relays.dischargeOff();
    } else if (strcmp(cmd, "relay_on") == 0) {
        relays.chargeOn();
        relays.dischargeOn();
    } else if (strcmp(cmd, "charge_on") == 0) {
        relays.chargeOn();
    } else if (strcmp(cmd, "discharge_on") == 0) {
        relays.dischargeOn();
    } else {
        Serial.printf("[CMD] Unknown command: %s\n", cmd);
    }
}

void connectWiFi() {
    Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WIFI] Connected, IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n[WIFI] Connection failed – running offline protection only");
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n========== BMS LiFePO4 4S ==========");

    relays.begin();
    oled.begin();

    if (!sensors.begin()) {
        Serial.println("[INIT] Sensor init failed – halting");
        while (true) delay(1000);
    }

    connectWiFi();
    mqtt.begin(onMqttCommand);
}

void loop() {
    mqtt.loop();

    unsigned long now = millis();

    if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        lastSensorRead = now;
        latestReadings = sensors.read();
        checkLocalProtection(latestReadings);
    }

    if (now - lastMqttPublish >= MQTT_PUBLISH_INTERVAL_MS) {
        lastMqttPublish = now;
        if (mqtt.isConnected() && latestReadings.valid) {
            mqtt.publishTelemetry(latestReadings);
        }
    }

    // Update OLED every 500ms
    if (now - lastOledUpdate >= 500) {
        lastOledUpdate = now;
        oled.update(latestReadings,
                    mqtt.isConnected(),
                    WiFi.status() == WL_CONNECTED,
                    relays.isChargeOn(),
                    relays.isDischargeOn());
    }

    // Reconnect WiFi if dropped
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastWifiRetry = 0;
        if (now - lastWifiRetry > 10000) {
            lastWifiRetry = now;
            WiFi.reconnect();
        }
    }
}

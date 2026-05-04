#include "relay_mqtt.h"
#include "globals.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

static String g_topicBase;
static String g_topicData;
static String g_topicStatus;
static String g_topicCmd;
static uint32_t g_lastMqttReconnectMs = 0;
static uint32_t g_lastMqttPublishMs = 0;

const char *relayModeStr(RelayMode mode) {
  switch (mode) {
    case RELAY_MODE_FORCE_CUT:
      return "force_cut";
    case RELAY_MODE_FORCE_CONNECT:
      return "force_connect";
    default:
      return "auto";
  }
}

static void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String topicStr = String(topic);
  if (topicStr != g_topicCmd) {
    return;
  }

  String cmd;
  cmd.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) {
    cmd += (char)payload[i];
  }
  cmd.toLowerCase();
  cmd.trim();

  if (cmd == "auto") {
    g_relayMode = RELAY_MODE_AUTO;
  } else if (cmd == "cut" || cmd == "off" || cmd == "0") {
    g_relayMode = RELAY_MODE_FORCE_CUT;
  } else if (cmd == "connect" || cmd == "on" || cmd == "1") {
    g_relayMode = RELAY_MODE_FORCE_CONNECT;
  } else {
    return;
  }

  String status = String("{\"event\":\"relay_mode\",\"mode\":\"") + relayModeStr(g_relayMode) + "\"}";
  mqttClient.publish(g_topicStatus.c_str(), status.c_str(), true);
}

void relayInitPinsEarly(void) {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_IDLE_LEVEL);
}

void relayApplyCut(bool cut) {
  g_relayCutApplied = cut;
  digitalWrite(RELAY_PIN, cut ? RELAY_ACTIVE_LEVEL : RELAY_IDLE_LEVEL);
}

void relayThermalUpdate(float tempC, bool tempValid) {
  if (tempValid) {
    if (!g_autoThermalCut && tempC > TEMP_CUT_CHARGE_C) {
      g_autoThermalCut = true;
    } else if (g_autoThermalCut && tempC < TEMP_RESTORE_CHARGE_C) {
      g_autoThermalCut = false;
    }
  }
}

bool relayUpdateOutput(void) {
  bool relayCutWanted = g_autoThermalCut;
  if (g_relayMode == RELAY_MODE_FORCE_CUT) {
    relayCutWanted = true;
  } else if (g_relayMode == RELAY_MODE_FORCE_CONNECT) {
    relayCutWanted = false;
  }
  relayApplyCut(relayCutWanted);
  return g_relayCutApplied;
}

void setupWifi(void) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi");
  uint32_t startMs = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startMs) < WIFI_CONNECT_TIMEOUT_MS) {
    Serial.print(".");
    delay(250);
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi STA IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connect timeout");
  }
}

void setupMqtt(void) {
  g_topicBase = String("iot/") + DEVICE_ID;
  g_topicData = g_topicBase + "/telemetry";
  g_topicStatus = g_topicBase + "/status";
  g_topicCmd = g_topicBase + "/relay/cmd";

  wifiClient.setInsecure();
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  Serial.print("MQTT ");
  Serial.print(MQTT_HOST);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  Serial.print("MQTT cmd topic: ");
  Serial.println(g_topicCmd);
}

void ensureWifi(void) {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t startMs = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startMs) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(200);
  }
}

void ensureMqtt(void) {
  if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
    uint32_t now = millis();
    if ((now - g_lastMqttReconnectMs) >= MQTT_RECONNECT_MS) {
      g_lastMqttReconnectMs = now;
      String clientId = String(DEVICE_ID) + "_" + String((uint32_t)ESP.getEfuseMac(), HEX);
      if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
        mqttClient.subscribe(g_topicCmd.c_str());
        mqttClient.publish(g_topicStatus.c_str(), "{\"event\":\"online\"}", true);
      }
    }
  }
}

void publishTelemetry(void) {
  if (!mqttClient.connected()) {
    return;
  }
  uint32_t now = millis();
  if ((now - g_lastMqttPublishMs) < MQTT_PUBLISH_MS) {
    return;
  }
  g_lastMqttPublishMs = now;

  String payload;
  payload.reserve(320);
  payload += "{";
  payload += "\"vbus\":" + String(g_lastVBus, 3) + ",";
  payload += "\"vavg\":" + String(g_lastVAvg, 3) + ",";
  payload += "\"current_mA\":" + String(g_lastCurrent, 1) + ",";
  payload += "\"temp_c\":" + String(g_lastTempC, 1) + ",";
  payload += "\"percent\":" + String(g_lastPercent, 1) + ",";
  payload += "\"charging\":" + String(g_lastCharging ? "true" : "false") + ",";
  payload += "\"temp_valid\":" + String(g_lastTempValid ? "true" : "false") + ",";
  payload += "\"relay_cut\":" + String(g_relayCutApplied ? "true" : "false") + ",";
  payload += "\"relay_mode\":\"" + String(relayModeStr(g_relayMode)) + "\"";
  payload += "}";

  mqttClient.publish(g_topicData.c_str(), payload.c_str(), false);
}

#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors.h"

typedef void (*CmdCallback)(const char* cmd, const char* reason);

class MqttHandler {
public:
    void begin(CmdCallback onCmd);
    void loop();
    bool isConnected();
    void publishTelemetry(const BmsReadings& r);
    void publishAlert(const char* type, const char* severity,
                      int cellIndex, float value, float threshold,
                      const char* action);

private:
    WiFiClient   _wifiClient;
    PubSubClient _mqtt{_wifiClient};
    CmdCallback  _cmdCb = nullptr;
    unsigned long _lastReconnect = 0;

    void _reconnect();
    static void _onMessage(char* topic, byte* payload, unsigned int len);
    static MqttHandler* _instance;
};

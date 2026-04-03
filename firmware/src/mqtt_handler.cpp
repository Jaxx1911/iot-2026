#include "mqtt_handler.h"

MqttHandler* MqttHandler::_instance = nullptr;

void MqttHandler::begin(CmdCallback onCmd) {
    _instance = this;
    _cmdCb = onCmd;
    _mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    _mqtt.setCallback(_onMessage);
    _mqtt.setBufferSize(512);
}

void MqttHandler::loop() {
    if (!_mqtt.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnect > MQTT_RECONNECT_INTERVAL_MS) {
            _lastReconnect = now;
            _reconnect();
        }
    }
    _mqtt.loop();
}

bool MqttHandler::isConnected() {
    return _mqtt.connected();
}

void MqttHandler::_reconnect() {
    String clientId = "bms-" + String(PACK_ID);
    Serial.printf("[MQTT] Connecting to %s:%d ...\n", MQTT_BROKER, MQTT_PORT);

    if (_mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
        Serial.println("[MQTT] Connected");
        _mqtt.subscribe(TOPIC_CMD);
    } else {
        Serial.printf("[MQTT] Failed, rc=%d\n", _mqtt.state());
    }
}

void MqttHandler::_onMessage(char* topic, byte* payload, unsigned int len) {
    if (!_instance || !_instance->_cmdCb) return;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);
    if (err) {
        Serial.printf("[MQTT] JSON parse error: %s\n", err.c_str());
        return;
    }

    const char* cmd    = doc["cmd"]    | "unknown";
    const char* reason = doc["reason"] | "";
    Serial.printf("[MQTT] CMD received: %s reason=%s\n", cmd, reason);
    _instance->_cmdCb(cmd, reason);
}

void MqttHandler::publishTelemetry(const BmsReadings& r) {
    JsonDocument doc;
    doc["pack_id"] = PACK_ID;
    doc["ts"]      = (unsigned long)(millis() / 1000);

    JsonArray cells = doc["cells"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_CELLS; i++)
        cells.add(serialized(String(r.cellVoltages[i], 3)));

    doc["v_pack"]  = serialized(String(r.packVoltage, 2));
    doc["current"] = serialized(String(r.current, 3));

    JsonArray temps = doc["temp"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_TEMP_SENSORS; i++)
        temps.add(serialized(String(r.temperatures[i], 1)));

    doc["soc"] = serialized(String(r.soc, 1));

    char buf[384];
    serializeJson(doc, buf, sizeof(buf));
    _mqtt.publish(TOPIC_TELEMETRY, buf);
}

void MqttHandler::publishAlert(const char* type, const char* severity,
                                int cellIndex, float value, float threshold,
                                const char* action) {
    JsonDocument doc;
    doc["pack_id"]      = PACK_ID;
    doc["type"]         = type;
    doc["severity"]     = severity;
    doc["cell_index"]   = cellIndex;
    doc["value"]        = serialized(String(value, 3));
    doc["threshold"]    = serialized(String(threshold, 3));
    doc["ts"]           = (unsigned long)(millis() / 1000);
    doc["action_taken"] = action;

    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    _mqtt.publish(TOPIC_ALERT, buf);
}

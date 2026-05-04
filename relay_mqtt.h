#pragma once

#include <Arduino.h>
#include "config.h"

void relayInitPinsEarly(void);
void relayApplyCut(bool cut);
const char *relayModeStr(RelayMode mode);
void relayThermalUpdate(float tempC, bool tempValid);
bool relayUpdateOutput(void);

void setupWifi(void);
void setupMqtt(void);
void ensureWifi(void);
void ensureMqtt(void);
void publishTelemetry(void);

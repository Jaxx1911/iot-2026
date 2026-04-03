#include "relay_control.h"

void RelayControl::begin() {
    pinMode(RELAY_CHARGE, OUTPUT);
    pinMode(RELAY_DISCHARGE, OUTPUT);
    // Default: both paths ON (relay energised = path closed)
    chargeOn();
    dischargeOn();
}

void RelayControl::chargeOn() {
    digitalWrite(RELAY_CHARGE, HIGH);
    _chargeState = true;
    Serial.println("[RELAY] Charge path ON");
}

void RelayControl::chargeOff() {
    digitalWrite(RELAY_CHARGE, LOW);
    _chargeState = false;
    Serial.println("[RELAY] Charge path OFF");
}

void RelayControl::dischargeOn() {
    digitalWrite(RELAY_DISCHARGE, HIGH);
    _dischargeState = true;
    Serial.println("[RELAY] Discharge path ON");
}

void RelayControl::dischargeOff() {
    digitalWrite(RELAY_DISCHARGE, LOW);
    _dischargeState = false;
    Serial.println("[RELAY] Discharge path OFF");
}

void RelayControl::allOff() {
    chargeOff();
    dischargeOff();
}

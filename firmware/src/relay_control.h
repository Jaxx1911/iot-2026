#pragma once
#include <Arduino.h>
#include "config.h"

class RelayControl {
public:
    void begin();
    void chargeOn();
    void chargeOff();
    void dischargeOn();
    void dischargeOff();
    void allOff();
    bool isChargeOn()    const { return _chargeState; }
    bool isDischargeOn() const { return _dischargeState; }

private:
    bool _chargeState    = false;
    bool _dischargeState = false;
};

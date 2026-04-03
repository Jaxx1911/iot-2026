#include "oled_display.h"

bool OledDisplay::begin() {
    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("[OLED] SSD1306 not found");
        return false;
    }
    Serial.println("[OLED] SSD1306 128x64 OK");
    showSplash();
    return true;
}

void OledDisplay::showSplash() {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setTextSize(2);
    _oled.setCursor(22, 8);
    _oled.print("BMS 4S");
    _oled.setTextSize(1);
    _oled.setCursor(16, 36);
    _oled.print("LiFePO4 Monitor");
    _oled.setCursor(28, 52);
    _oled.print("Booting...");
    _oled.display();
}

void OledDisplay::update(const BmsReadings& r, bool mqttOk, bool wifiOk,
                          bool chargeOn, bool dischargeOn) {
    unsigned long now = millis();
    if (now - _lastPageSwitch > OLED_PAGE_INTERVAL_MS) {
        _lastPageSwitch = now;
        _page = (_page + 1) % 2;
    }

    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    if (_page == 0)
        _drawPage0(r, mqttOk, wifiOk);
    else
        _drawPage1(r, chargeOn, dischargeOn);

    _oled.display();
}

// Page 0: Pack overview – Vpack, SOC, Current, Temp, status icons
void OledDisplay::_drawPage0(const BmsReadings& r, bool mqttOk, bool wifiOk) {
    // Status bar
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(wifiOk ? "WiFi" : "----");
    _oled.setCursor(40, 0);
    _oled.print(mqttOk ? "MQTT" : "----");
    _oled.setCursor(80, 0);
    _oled.printf("SOC:%d%%", (int)r.soc);

    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // Pack voltage (large)
    _oled.setTextSize(2);
    _oled.setCursor(0, 14);
    _oled.printf("%5.2fV", r.packVoltage);

    // Current
    _oled.setTextSize(1);
    _oled.setCursor(0, 36);
    _oled.printf("I: %+.2fA", r.current);

    // Temperatures
    _oled.setCursor(0, 48);
    _oled.printf("T1:%.1f", r.temperatures[0]);
    _oled.setCursor(64, 48);
    _oled.printf("T2:%.1f", r.temperatures[1]);

    // SOC bar (bottom)
    int barW = (int)(r.soc / 100.0f * 124);
    _oled.drawRect(2, 58, 124, 6, SSD1306_WHITE);
    _oled.fillRect(2, 58, barW, 6, SSD1306_WHITE);
}

// Page 1: Cell voltages + relay status
void OledDisplay::_drawPage1(const BmsReadings& r, bool chargeOn, bool dischargeOn) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print("CELL VOLTAGES");
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        int y = 14 + i * 10;
        _oled.setCursor(0, y);
        _oled.printf("C%d: %.3fV", i + 1, r.cellVoltages[i]);

        // Visual bar (scaled 2.5V–3.65V → 0–50px)
        float pct = (r.cellVoltages[i] - 2.5f) / (3.65f - 2.5f);
        pct = constrain(pct, 0.0f, 1.0f);
        int bw = (int)(pct * 50);
        _oled.fillRect(76, y, bw, 7, SSD1306_WHITE);
        _oled.drawRect(76, y, 50, 7, SSD1306_WHITE);
    }

    // Relay status
    _oled.setCursor(0, 56);
    _oled.printf("CHG:%s  DIS:%s",
                 chargeOn ? "ON " : "OFF",
                 dischargeOn ? "ON " : "OFF");
}

void OledDisplay::showAlert(const char* alertType, int cellIndex, float value) {
    _oled.clearDisplay();
    _oled.setTextSize(2);
    _oled.setCursor(10, 0);
    _oled.print("! ALERT !");
    _oled.setTextSize(1);
    _oled.setCursor(0, 24);
    _oled.print(alertType);
    if (cellIndex >= 0) {
        _oled.setCursor(0, 38);
        _oled.printf("Cell %d: %.3f", cellIndex + 1, value);
    } else {
        _oled.setCursor(0, 38);
        _oled.printf("Value: %.3f", value);
    }
    _oled.setCursor(0, 54);
    _oled.print("Protection ACTIVE");
    _oled.display();
}

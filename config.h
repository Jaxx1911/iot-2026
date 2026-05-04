#pragma once

#include <Arduino.h>

// Phải khai báo trước prototype Arduino tự sinh (RelayMode trong tham số hàm).
enum RelayMode {
  RELAY_MODE_AUTO = 0,
  RELAY_MODE_FORCE_CUT = 1,
  RELAY_MODE_FORCE_CONNECT = 2,
};

#define BATTERY_NOMINAL_V   3.70f
#define BATTERY_FULL_V      4.20f
#define BATTERY_CAPACITY_MAH 2000

#define MA_VOLTAGE_SAMPLES 10

#define CHARGE_I_MA_MIN         10.0f
#define CHARGE_V_OFFSET         0.04f
#define CHARGE_BU_APPLY_ABOVE_V 3.50f
#define HEAVY_LOAD_I_MA         400.0f
#define VBUS_PLAUSIBLE_LOW_V    2.50f

#define TEMP_CUT_CHARGE_C      45.0f
#define TEMP_RESTORE_CHARGE_C  42.0f

#define RELAY_PIN            26
#define RELAY_ACTIVE_LEVEL   HIGH
#define RELAY_IDLE_LEVEL     LOW

// WiFi/MQTT: điền trong secrets.h (bản sao của secrets.example.h — file đó không commit).
#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000;
constexpr uint16_t MQTT_PORT = 8883;
constexpr uint32_t MQTT_RECONNECT_MS = 3000;
constexpr uint32_t MQTT_PUBLISH_MS = 5000;

static const char *const DEVICE_ID = "esp32_bat_001";

#define TEMP_SMOOTH_ALPHA       0.20f
#define PERCENT_MAX_SLEW_PER_S  1.5f
#define MODE_STABLE_LOOPS       2
#define INA219_CURRENT_SIGN     (1.0f)

#define ONE_WIRE_BUS 4

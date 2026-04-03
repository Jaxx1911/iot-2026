#pragma once

// ── WiFi ──
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ── MQTT ──
#define MQTT_BROKER   "192.168.1.100"
#define MQTT_PORT     1883
#define MQTT_USER     ""
#define MQTT_PASS     ""
#define PACK_ID       "pack01"

#define TOPIC_TELEMETRY "bms/" PACK_ID "/telemetry"
#define TOPIC_ALERT     "bms/" PACK_ID "/alert"
#define TOPIC_CMD       "bms/" PACK_ID "/cmd"

// ── Sensor pins ──
#define ONE_WIRE_PIN  4
#define RELAY_CHARGE  16
#define RELAY_DISCHARGE 17

// ── Thresholds (LiFePO4) ──
#define CELL_OV_THRESHOLD   3.65f   // over-voltage per cell (V)
#define CELL_UV_THRESHOLD   2.50f   // under-voltage per cell (V)
#define PACK_OC_THRESHOLD   10.0f   // over-current (A)
#define TEMP_OT_THRESHOLD   45.0f   // over-temperature (°C)

// ── Timing ──
#define SENSOR_READ_INTERVAL_MS   500
#define MQTT_PUBLISH_INTERVAL_MS  2000
#define MQTT_RECONNECT_INTERVAL_MS 5000

// ── Cell configuration ──
#define NUM_CELLS 4
#define NUM_TEMP_SENSORS 2

// Voltage divider ratio: Rtop=20k, Rbot=10k → ratio = (20+10)/10 = 3.0
#define VDIV_RATIO 3.0f

// ── OLED Display ──
#define OLED_PAGE_INTERVAL_MS 4000

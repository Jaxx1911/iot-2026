# Kiến trúc hệ thống BMS cho năng lượng tái tạo

## Tổng quan

Hệ thống BMS (Battery Management System) giám sát pack pin LiFePO4 4S dùng cho
lưu trữ năng lượng tái tạo. Kiến trúc chia 5 lớp: Edge/IoT, Message Broker,
Backend, Database và Frontend.

## Sơ đồ kiến trúc

```
┌──────────────────────────────────────────────────────────────────┐
│                        PHYSICAL LAYER                            │
│  ┌────────────┐  ┌──────────┐  ┌───────────┐  ┌──────────────┐  │
│  │ LiFePO4 4S │──│ ADS1115  │──│  INA219   │──│  DS18B20     │  │
│  │ Battery    │  │ (Vcell)  │  │ (I, Vbus) │  │  (Temp)      │  │
│  └──────┬─────┘  └────┬─────┘  └─────┬─────┘  └──────┬───────┘  │
│         │              │ I2C          │ I2C           │ OneWire  │
│  ┌──────┴─────┐  ┌────┴──────────────┴───────────────┴────┐     │
│  │Solar Panel │  │            ESP32 DevKit                 │     │
│  │ 6V 1W/5W  │  │  - Sensor read loop (500ms)             │     │
│  │ + charger  │  │  - Local threshold check                │     │
│  └────────────┘  │  - MQTT publish / subscribe             │     │
│                  │  - Relay/MOSFET control (GPIO)          │     │
│  ┌────────────┐  │  - OLED SSD1306 local display (I2C)    │     │
│  │OLED 0.96"  │──│                                         │     │
│  │SSD1306 I2C │  └─────────────────┬──────────────────────┘     │
│  └────────────┘                    │ WiFi                        │
└───────────────────────────────────┼─────────────────────────────┘
                                    │
┌───────────────────────────────────┼─────────────────────────────┐
│                      MQTT BROKER (EMQX / Mosquitto)             │
│                                                                  │
│  Topics:                                                         │
│    bms/{packId}/telemetry   ← ESP32 publishes sensor data       │
│    bms/{packId}/alert       ← Backend publishes alerts          │
│    bms/{packId}/cmd         ← Backend publishes control cmds    │
│                               ESP32 subscribes                   │
└───────────────────────────────────┬─────────────────────────────┘
                                    │
┌───────────────────────────────────┼─────────────────────────────┐
│                         BACKEND (FastAPI)                        │
│  ┌───────────────┐  ┌────────────────┐  ┌───────────────────┐   │
│  │ MQTT Client   │──│ Rule Engine    │──│ REST API          │   │
│  │ (subscribe +  │  │ (thresholds,   │  │ (GET telemetry,   │   │
│  │  publish cmd) │  │  SOC estimate) │  │  POST config/cmd) │   │
│  └───────┬───────┘  └───────┬────────┘  └────────┬──────────┘   │
│          │                  │                     │              │
│          └──────────────────┼─────────────────────┘              │
│                             │                                    │
│                    ┌────────┴────────┐                           │
│                    │    InfluxDB     │                           │
│                    │  (time-series)  │                           │
│                    └─────────────────┘                           │
└──────────────────────────────────────────────────────────────────┘
                                    │
┌───────────────────────────────────┼─────────────────────────────┐
│                    FRONTEND (React + Vite)                       │
│  ┌────────────┐ ┌────────────┐ ┌──────────┐ ┌───────────────┐   │
│  │ Dashboard  │ │ Alert      │ │ Control  │ │ History       │   │
│  │ (realtime) │ │ Panel      │ │ Panel    │ │ Charts        │   │
│  └────────────┘ └────────────┘ └──────────┘ └───────────────┘   │
│                                                                  │
│  WebSocket / Polling ← Backend REST + SSE                       │
└──────────────────────────────────────────────────────────────────┘
```

## Giao tiếp giữa các lớp

| Từ       | Đến      | Giao thức       | Dữ liệu                                |
|----------|----------|----------------|-----------------------------------------|
| ESP32    | Broker   | MQTT QoS 1     | JSON telemetry (Vcell[], I, T, SOC)     |
| Broker   | Backend  | MQTT subscribe | Telemetry stream                        |
| Backend  | InfluxDB | HTTP write API | Point (measurement, tags, fields, time) |
| Backend  | Broker   | MQTT publish   | JSON command (relay_on/off, reset)      |
| Broker   | ESP32    | MQTT subscribe | Command payload                         |
| Backend  | Frontend | REST + SSE     | JSON API responses / event stream       |
| Frontend | Backend  | REST POST      | Control commands, config updates        |

## MQTT Payload Definitions

### Telemetry (ESP32 → Backend)
```json
{
  "pack_id": "pack01",
  "ts": 1711900000,
  "cells": [3.32, 3.31, 3.33, 3.30],
  "v_pack": 13.26,
  "current": 2.15,
  "temp": [28.5, 29.0],
  "soc": 78.5
}
```

### Command (Backend → ESP32)
```json
{
  "pack_id": "pack01",
  "cmd": "relay_off",
  "reason": "over_voltage",
  "ts": 1711900005
}
```

### Alert (Backend → FE via REST/SSE)
```json
{
  "pack_id": "pack01",
  "type": "over_voltage",
  "severity": "critical",
  "cell_index": 2,
  "value": 3.72,
  "threshold": 3.65,
  "ts": 1711900005,
  "action_taken": "relay_off"
}
```

## Cấu hình pin LiFePO4 4S

| Thông số              | Giá trị           |
|-----------------------|-------------------|
| Số cell nối tiếp      | 4S                |
| Điện áp nominal/cell  | 3.2V              |
| Điện áp pack nominal  | 12.8V             |
| Ngưỡng sạc đầy/cell  | 3.65V             |
| Ngưỡng xả hết/cell   | 2.5V              |
| Dòng max (demo)       | 5A                |
| Nhiệt độ hoạt động    | 0°C – 45°C       |

## Thành phần bổ sung

### OLED Display SSD1306 128x64

Màn hình OLED 0.96 inch giao tiếp I2C (địa chỉ 0x3C), hiển thị trạng thái
BMS ngay tại chỗ mà không cần mở dashboard web. Gồm 2 trang tự động xoay:

- **Trang 0:** Vpack (lớn), SOC bar, dòng điện, 2 nhiệt độ, trạng thái WiFi/MQTT
- **Trang 1:** Điện áp 4 cell (số + thanh bar), trạng thái Charge/Discharge relay
- **Trang Alert:** Khi có sự cố, hiển thị loại lỗi + cell index + giá trị vi phạm

### Tấm pin mặt trời mini

Solar Panel 6V 1W (hoặc 5W) polycrystalline là nguồn năng lượng tái tạo thực
cho demo. Kết nối qua module sạc TP4056 hoặc CN3791 (MPPT mini) để sạc pack
LiFePO4 4S. Diode Schottky 1N5819 ngăn dòng ngược từ pin về tấm solar khi
không có ánh sáng.

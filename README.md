# BMS - Battery Management System for Renewable Energy

Hệ thống quản lý pin (BMS) cho lưu trữ năng lượng tái tạo.
Đồ án bài tập lớn môn **Xây dựng hệ thống nhúng**.

## Architecture

```
ESP32 + Sensors ──MQTT──▶ Mosquitto ──▶ FastAPI Backend ──▶ InfluxDB
                                              │
                                     React Dashboard (Vite)
```

| Layer     | Technology                                      |
|-----------|-------------------------------------------------|
| IoT/Edge  | ESP32, ADS1115, INA219, DS18B20, OLED SSD1306   |
| Solar     | Solar Panel 6V 1W/5W + TP4056/CN3791 charger    |
| Broker    | Eclipse Mosquitto (MQTT)                         |
| Backend   | Python FastAPI + Rule Engine                     |
| Database  | InfluxDB 2 (time-series)                         |
| Frontend  | React 18 + Vite + Recharts                       |
| Simulator | Python script (no hardware needed)               |

## Project Structure

```
├── firmware/           ESP32 PlatformIO project
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp
│       ├── config.h
│       ├── sensors.h / .cpp
│       ├── mqtt_handler.h / .cpp
│       └── relay_control.h / .cpp
├── backend/            FastAPI backend
│   ├── Dockerfile
│   ├── requirements.txt
│   └── app/
│       ├── main.py
│       ├── config.py
│       ├── routes.py
│       ├── mqtt_client.py
│       ├── rule_engine.py
│       ├── influx.py
│       └── models.py
├── frontend/           React dashboard
│   ├── Dockerfile
│   ├── package.json
│   └── src/
│       ├── App.tsx
│       ├── api/
│       ├── components/
│       └── types/
├── simulator/          Python MQTT telemetry simulator
│   ├── bms_simulator.py
│   └── test_fault_scenarios.py
├── mosquitto/          Mosquitto broker config
├── docs/               Architecture & BOM documentation
│   ├── architecture.md
│   └── bom.md
└── docker-compose.yml  Full stack deployment
```

## Quick Start (Demo without hardware)

### Prerequisites
- Docker & Docker Compose
- Python 3.10+ (for simulator)
- Node.js 18+ (for local frontend dev)

### 1. Start infrastructure

```bash
docker compose up -d
```

This starts: Mosquitto (port 1883), InfluxDB (port 8086), Backend (port 8000), Frontend (port 3000).

### 2. Run the simulator

```bash
cd simulator
pip install -r requirements.txt
python bms_simulator.py --broker localhost --port 1883
```

### 3. Open the dashboard

Open http://localhost:3000 in your browser.

### 4. Run fault test scenarios

```bash
python simulator/test_fault_scenarios.py --broker localhost
```

## Local Development

### Backend (without Docker)

```bash
cd backend
pip install -r requirements.txt
uvicorn app.main:app --reload --port 8000
```

### Frontend (without Docker)

```bash
cd frontend
npm install
npm run dev
```

The Vite dev server proxies `/api` to `http://localhost:8000`.

### Firmware

Open `firmware/` in PlatformIO IDE or VS Code with PlatformIO extension.

1. Edit `firmware/src/config.h` with your WiFi credentials and MQTT broker IP.
2. Build and flash to ESP32.

## API Endpoints

| Method | Path                    | Description                    |
|--------|-------------------------|--------------------------------|
| GET    | /api/telemetry/latest   | Latest telemetry snapshot      |
| GET    | /api/telemetry/history  | Historical data (query params) |
| GET    | /api/alerts/latest      | Recent in-memory alerts        |
| GET    | /api/alerts/history     | Historical alerts from DB      |
| POST   | /api/control/command    | Send command to ESP32          |
| GET    | /api/config/thresholds  | Current threshold config       |
| PUT    | /api/config/thresholds  | Update thresholds at runtime   |
| GET    | /api/health             | Health check                   |

## MQTT Topics

| Topic                       | Publisher | Subscriber | Payload        |
|-----------------------------|-----------|------------|----------------|
| `bms/{packId}/telemetry`    | ESP32     | Backend    | JSON telemetry |
| `bms/{packId}/alert`        | Backend   | FE (opt)   | JSON alert     |
| `bms/{packId}/cmd`          | Backend   | ESP32      | JSON command   |

## Protection Rules

| Rule             | Threshold  | Action          |
|------------------|-----------|-----------------|
| Over-voltage     | > 3.65V   | Charge OFF      |
| Under-voltage    | < 2.50V   | Discharge OFF   |
| Over-current     | > 10A     | All OFF         |
| Over-temperature | > 45°C    | All OFF         |
| Cell imbalance   | > 0.10V   | Warning only    |

## Documentation

| Document | Description |
|----------|-------------|
| [docs/architecture.md](docs/architecture.md) | System architecture, layer diagram, MQTT payload definitions |
| [docs/bom.md](docs/bom.md) | Bill of materials with part names for online search (~860,000 VND) |
| [docs/hardware-wiring-guide.md](docs/hardware-wiring-guide.md) | Pin-by-pin wiring for ESP32, ADS1115, INA219, DS18B20, Relay |
| [docs/software-setup-guide.md](docs/software-setup-guide.md) | Docker / local setup for MQTT, InfluxDB, Backend, Frontend, Firmware |
| [docs/troubleshooting.md](docs/troubleshooting.md) | FAQ and fix for common MQTT, sensor, Docker, network issues |

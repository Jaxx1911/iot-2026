# Huong dan cai dat va chay phan mem (Software Setup Guide)

## Tong quan

Tai lieu nay huong dan chi tiet cach cai dat va chay tung thanh phan phan mem:
Firmware ESP32, MQTT Broker, Backend, InfluxDB, Frontend va Simulator.

Co 2 cach chay:
- **Cach 1 (Docker):** Nhanh nhat, chay toan bo bang `docker compose up`
- **Cach 2 (Thu cong):** Chay tung thanh phan rieng le, phu hop khi phat trien

---

## Yeu cau he thong

| Phan mem         | Phien ban toi thieu | Muc dich                      |
|------------------|---------------------|-------------------------------|
| Docker Desktop   | 24.x                | Chay MQTT, InfluxDB, BE, FE   |
| Python           | 3.10+               | Backend (local), Simulator    |
| Node.js          | 18+                 | Frontend (local dev)          |
| PlatformIO CLI hoac VS Code + PlatformIO | 6.x | Build & flash firmware |
| Git              | 2.x                 | Quan ly source                |

---

## Cach 1: Chay toan bo bang Docker Compose

### Buoc 1: Clone va di chuyen vao thu muc du an

```bash
cd d:\E\PersonalProject\iot-2026
```

### Buoc 2: Khoi dong tat ca services

```bash
docker compose up -d
```

Lenh nay se build va khoi dong:

| Service    | Port  | URL                          |
|------------|-------|------------------------------|
| Mosquitto  | 1883  | mqtt://localhost:1883        |
| Mosquitto WS | 9001 | ws://localhost:9001         |
| InfluxDB   | 8086  | http://localhost:8086        |
| Backend    | 8000  | http://localhost:8000        |
| Frontend   | 3000  | http://localhost:3000        |

### Buoc 3: Kiem tra trang thai

```bash
docker compose ps
```

Tat ca services phai o trang thai "Up" hoac "running".

### Buoc 4: Xem log

```bash
# Xem log tat ca
docker compose logs -f

# Xem log rieng tung service
docker compose logs -f backend
docker compose logs -f mosquitto
```

### Buoc 5: Dung tat ca

```bash
docker compose down

# Xoa ca du lieu (InfluxDB, Mosquitto)
docker compose down -v
```

---

## Cach 2: Chay thu cong tung thanh phan

### 2.1. MQTT Broker (Mosquitto)

**Cach A: Dung Docker rieng**

```bash
docker run -d --name mosquitto -p 1883:1883 -p 9001:9001 ^
  -v %cd%/mosquitto/config:/mosquitto/config ^
  eclipse-mosquitto:2
```

**Cach B: Cai truc tiep tren Windows**

1. Tai Mosquitto tu https://mosquitto.org/download/
2. Cai dat va them vao PATH
3. Tao file config:
   ```
   listener 1883
   allow_anonymous true
   ```
4. Chay: `mosquitto -c mosquitto.conf -v`

**Kiem tra MQTT hoat dong:**

```bash
# Terminal 1 - Subscribe
mosquitto_sub -h localhost -t "bms/#" -v

# Terminal 2 - Publish test
mosquitto_pub -h localhost -t "bms/pack01/test" -m "hello"
```

Neu Terminal 1 hien `bms/pack01/test hello` la thanh cong.

---

### 2.2. InfluxDB

**Cach A: Docker rieng**

```bash
docker run -d --name influxdb -p 8086:8086 ^
  -e DOCKER_INFLUXDB_INIT_MODE=setup ^
  -e DOCKER_INFLUXDB_INIT_USERNAME=admin ^
  -e DOCKER_INFLUXDB_INIT_PASSWORD=admin12345 ^
  -e DOCKER_INFLUXDB_INIT_ORG=bms-org ^
  -e DOCKER_INFLUXDB_INIT_BUCKET=bms ^
  -e DOCKER_INFLUXDB_INIT_ADMIN_TOKEN=bms-secret-token ^
  influxdb:2
```

**Kiem tra InfluxDB:**

Mo http://localhost:8086 trong trinh duyet.
Dang nhap voi `admin` / `admin12345`.
Vao Data Explorer > chon bucket `bms` de xem du lieu.

**Cach B: Cai truc tiep**

1. Tai tu https://portal.influxdata.com/downloads/
2. Giai nen va chay: `influxd`
3. Mo http://localhost:8086 va setup:
   - Username: `admin`
   - Password: `admin12345`
   - Org: `bms-org`
   - Bucket: `bms`
4. Tao API token va cap nhat vao `backend/app/config.py` hoac bien moi truong

---

### 2.3. Backend (FastAPI)

```bash
cd backend

# Tao virtual environment (khuyen nghi)
python -m venv venv

# Kich hoat (Windows PowerShell)
.\venv\Scripts\Activate.ps1

# Kich hoat (Windows CMD)
venv\Scripts\activate.bat

# Cai dependencies
pip install -r requirements.txt
```

**Cau hinh bien moi truong (PowerShell):**

```powershell
$env:MQTT_BROKER = "localhost"
$env:MQTT_PORT = "1883"
$env:INFLUXDB_URL = "http://localhost:8086"
$env:INFLUXDB_TOKEN = "bms-secret-token"
$env:INFLUXDB_ORG = "bms-org"
$env:INFLUXDB_BUCKET = "bms"
```

Hoac tao file `.env` trong thu muc `backend/`:

```env
MQTT_BROKER=localhost
MQTT_PORT=1883
INFLUXDB_URL=http://localhost:8086
INFLUXDB_TOKEN=bms-secret-token
INFLUXDB_ORG=bms-org
INFLUXDB_BUCKET=bms
```

**Chay backend:**

```bash
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

**Kiem tra backend:**

```bash
# Health check
curl http://localhost:8000/api/health

# Xem API docs tu dong (Swagger)
# Mo trinh duyet: http://localhost:8000/docs
```

FastAPI tu dong tao Swagger UI tai http://localhost:8000/docs de thu nghiem API.

---

### 2.4. Frontend (React Dashboard)

```bash
cd frontend

# Cai dependencies
npm install

# Chay dev server
npm run dev
```

Dev server chay tai http://localhost:5173 (Vite dev) va tu dong proxy `/api`
den `http://localhost:8000`.

**Build production:**

```bash
npm run build
```

Output nam trong `frontend/dist/`, co the serve bang bat ky HTTP server nao.

---

### 2.5. Simulator (Khong can phan cung)

Simulator gia lap ESP32 gui du lieu qua MQTT de test toan bo he thong.

```bash
cd simulator
pip install -r requirements.txt

# Chay simulator co ban
python bms_simulator.py --broker localhost --port 1883 --interval 2

# Chay kich ban loi
python test_fault_scenarios.py --broker localhost --port 1883
```

**Output mau tu simulator:**

```
Simulator publishing to localhost:1883 topic=bms/pack01/telemetry
Interval: 2.0s  |  Ctrl+C to stop

[1711900000] SOC=80.2% Vpack=13.24V I=2.156A T=[25.3, 26.1]
[1711900002] SOC=80.3% Vpack=13.25V I=1.892A T=[25.5, 26.3]
...
```

---

## Firmware ESP32

### Cai dat PlatformIO

**Cach A: VS Code Extension**

1. Mo VS Code
2. Vao Extensions, tim "PlatformIO IDE"
3. Cai dat va khoi dong lai VS Code

**Cach B: PlatformIO CLI**

```bash
pip install platformio
```

### Cau hinh truoc khi flash

Mo file `firmware/src/config.h` va sua cac thong so:

```cpp
// WiFi cua ban
#define WIFI_SSID     "Ten_WiFi_Cua_Ban"
#define WIFI_PASSWORD "Mat_Khau_WiFi"

// IP may chay MQTT Broker
// (dung ipconfig tren Windows de tim IP LAN)
#define MQTT_BROKER   "192.168.1.100"
#define MQTT_PORT     1883
```

**Tim IP may tinh (Windows):**

```powershell
ipconfig | Select-String "IPv4"
```

Lay dia chi IPv4 (vd: 192.168.1.100) dien vao MQTT_BROKER.

### Build va flash

**Voi VS Code + PlatformIO:**

1. Mo thu muc `firmware/` trong VS Code
2. Ket noi ESP32 qua USB
3. Nhan nut Build (dau tick) tren thanh PlatformIO
4. Nhan nut Upload (mui ten phai)
5. Mo Serial Monitor (baud 115200) de xem log

**Voi PlatformIO CLI:**

```bash
cd firmware

# Build
pio run

# Flash
pio run --target upload

# Xem Serial Monitor
pio device monitor --baud 115200
```

### Log mau khi ESP32 chay binh thuong

```
========== BMS LiFePO4 4S ==========
[RELAY] Charge path ON
[RELAY] Discharge path ON
[OLED] SSD1306 128x64 OK
[SENSOR] Found 2 DS18B20 sensor(s)
[WIFI] Connecting to MyWiFi.......
[WIFI] Connected, IP: 192.168.1.50
[MQTT] Connecting to 192.168.1.100:1883 ...
[MQTT] Connected
```

Man hinh OLED se hien splash "BMS 4S / LiFePO4 Monitor / Booting..." roi tu dong
chuyen sang hien thi du lieu realtime sau khi khoi dong xong.

### Xu ly loi thuong gap

| Loi                              | Nguyen nhan                    | Cach sua                           |
|----------------------------------|--------------------------------|------------------------------------|
| `[SENSOR] ADS1115 not found`    | Loi I2C hoac chua cam ADS1115 | Kiem tra day SDA/SCL, dia chi I2C  |
| `[SENSOR] INA219 not found`     | Loi I2C hoac chua cam INA219  | Kiem tra day, thu doi dia chi      |
| `[OLED] SSD1306 not found`      | Loi I2C hoac sai dia chi OLED | Thu 0x3C hoac 0x3D, kiem tra day   |
| `[WIFI] Connection failed`      | Sai SSID/password hoac xa WiFi| Kiem tra config.h, thu mang khac   |
| `[MQTT] Failed, rc=-2`          | Khong ket noi duoc broker     | Kiem tra IP broker, firewall       |
| `Found 0 DS18B20 sensor(s)`     | Thieu pull-up hoac loi day    | Kiem tra dien tro 4.7kOhm         |

---

## Thu tu khoi dong khuyen nghi

Khi chay thu cong (khong Docker), khoi dong theo thu tu sau:

```
1. Mosquitto (MQTT Broker)     ← Khoi dong dau tien
         ↓
2. InfluxDB                    ← Can truoc Backend
         ↓
3. Backend (FastAPI)           ← Ket noi MQTT + InfluxDB
         ↓
4. Frontend (React)            ← Goi API tu Backend
         ↓
5. Simulator hoac ESP32        ← Gui du lieu vao MQTT
```

Neu dung Docker Compose, thu tu nay duoc xu ly tu dong qua `depends_on`.

---

## Kiem tra toan bo he thong (End-to-End)

### Buoc 1: Khoi dong infrastructure

```bash
# Docker
docker compose up -d mosquitto influxdb backend frontend
```

### Buoc 2: Chay simulator

```bash
cd simulator
python bms_simulator.py --broker localhost
```

### Buoc 3: Kiem tra dashboard

Mo http://localhost:3000 (Docker) hoac http://localhost:5173 (Vite dev).
Sau 2-3 giay, dashboard phai hien thi:
- Pack Voltage (khoang 12.8V)
- SOC (thay doi tu tu)
- Current (duong khi sac, am khi xa)
- Temperature (khoang 25-27 do)
- 4 thanh cell voltage

### Buoc 4: Test dieu khien

Nhan nut "All OFF" tren dashboard.
Kiem tra terminal simulator/ESP32 co nhan duoc lenh `relay_off`.

### Buoc 5: Test canh bao

```bash
python simulator/test_fault_scenarios.py --broker localhost
```

Dashboard phai hien thi cac alert: over_voltage, under_voltage, over_current,
over_temperature, cell_imbalance.

### Buoc 6: Kiem tra InfluxDB

Mo http://localhost:8086, vao Data Explorer:
- Chon bucket: `bms`
- Chon measurement: `telemetry`
- Graph phai hien thi du lieu theo thoi gian

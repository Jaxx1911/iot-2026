# Xu ly su co va FAQ (Troubleshooting Guide)

## 1. Van de MQTT

### 1.1. Khong ket noi duoc broker

**Trieu chung:** Backend log `[MQTT] Connection error`, ESP32 log `rc=-2`

**Kiem tra:**

```bash
# Kiem tra Mosquitto dang chay
docker compose ps mosquitto

# Test ket noi bang mosquitto_pub
mosquitto_pub -h localhost -p 1883 -t "test" -m "hello"

# Kiem tra port dang lang nghe (PowerShell)
netstat -an | Select-String "1883"
```

**Nguyen nhan va cach sua:**

| Nguyen nhan              | Cach sua                                          |
|--------------------------|---------------------------------------------------|
| Mosquitto chua khoi dong | `docker compose up -d mosquitto`                  |
| Port bi chiem            | Tat ung dung khac dung port 1883                  |
| Firewall chan             | Mo port 1883 trong Windows Firewall               |
| Sai IP (tu ESP32)        | Kiem tra `ipconfig` va cap nhat `config.h`        |

### 1.2. ESP32 mat ket noi MQTT lien tuc

**Trieu chung:** Log lien tuc `[MQTT] Connecting...` roi `Failed`

**Kiem tra:**
- ESP32 va may chu MQTT co cung mang WiFi khong?
- Thu ping tu may tinh den IP ESP32 (xem log WiFi)
- Thu dung dien thoai lam hotspot de test mang don gian hon

### 1.3. Du lieu khong gui len duoc

**Trieu chung:** Simulator chay nhung dashboard khong co du lieu

```bash
# Subscribe truc tiep de kiem tra message di qua broker khong
mosquitto_sub -h localhost -t "bms/#" -v
```

Neu khong thay message nao → van de o publisher (simulator/ESP32).
Neu thay message → van de o backend subscriber.

---

## 2. Van de Backend

### 2.1. Backend khong khoi dong

**Trieu chung:** Loi import hoac port da duoc su dung

```bash
# Kiem tra loi chi tiet
docker compose logs backend

# Kiem tra port 8000
netstat -an | Select-String "8000"
```

**Cac loi thuong gap:**

| Loi                                    | Cach sua                              |
|----------------------------------------|---------------------------------------|
| `ModuleNotFoundError: paho.mqtt`       | `pip install -r requirements.txt`     |
| `Address already in use: port 8000`    | Tat process cu: `taskkill /F /PID ...`|
| `Connection refused` (InfluxDB)        | Kiem tra InfluxDB da khoi dong chua   |

### 2.2. API tra ve `{"status": "no_data"}`

**Nguyen nhan:** Backend chua nhan duoc telemetry nao tu MQTT.

**Kiem tra:**
1. Simulator hoac ESP32 dang chay?
2. MQTT broker dang chay?
3. Backend da subscribe thanh cong? (xem log: `[MQTT] Connected`)

### 2.3. Loi ghi InfluxDB

**Trieu chung:** Log `[INFLUX] Write error`

**Kiem tra:**
- InfluxDB dang chay: `docker compose ps influxdb`
- Token dung: so sanh `INFLUXDB_TOKEN` trong bien moi truong voi token trong InfluxDB UI
- Bucket ton tai: vao InfluxDB UI > Load Data > Buckets > tim `bms`

---

## 3. Van de Frontend

### 3.1. Dashboard trang (khong hien du lieu)

**Kiem tra:**

```bash
# API co tra du lieu khong?
curl http://localhost:8000/api/telemetry/latest

# Neu tra {"status": "no_data"} → van de o backend/MQTT
# Neu tra du lieu → van de o frontend
```

**Cac loi thuong gap:**

| Trieu chung                         | Cach sua                                  |
|-------------------------------------|-------------------------------------------|
| "Waiting for telemetry data..."     | Chua co du lieu, chay simulator truoc     |
| Console loi CORS                    | Kiem tra backend co CORSMiddleware khong  |
| Console loi 404 /api/...            | Kiem tra proxy trong vite.config.ts       |
| Trang trang hoan toan               | Mo Console (F12), kiem tra loi JS         |

### 3.2. Bieu do khong hien

**Nguyen nhan:** Khong co du lieu lich su trong InfluxDB.

Bieu do History can du lieu ton tai it nhat vai phut. Chay simulator khoang 5 phut
roi reload trang.

---

## 4. Van de Phan cung

### 4.1. ADS1115 khong doc duoc

**Trieu chung:** `[SENSOR] ADS1115 not found`

**Kiem tra:**
1. Day SDA (GPIO 21) va SCL (GPIO 22) dung chua?
2. Chan ADDR cua ADS1115 noi GND chua? (dia chi 0x48)
3. Co cap 3V3 va GND cho module khong?

**Quet bus I2C:**

Them doan code tam vao `setup()` de quet:

```cpp
#include <Wire.h>

void scanI2C() {
    Wire.begin(21, 22);
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C device found at 0x%02X\n", addr);
        }
    }
}
```

Phai thay:
- `0x3C` = OLED SSD1306
- `0x40` = INA219
- `0x48` = ADS1115

Neu khong thay gi → kiem tra day ket noi.

### 4.2. DS18B20 khong doc duoc

**Trieu chung:** `Found 0 DS18B20 sensor(s)` hoac doc -127.0°C

**Kiem tra:**
1. Dien tro pull-up 4.7kOhm giua Data va 3V3?
2. Day mau dung? (Do = Data, Den = GND, Vang = VCC)
3. Thu thay cam bien khac (co the bi hong)

### 4.3. INA219 doc sai dien ap/dong

**Kiem tra:**
- So sanh voi dong ho van nang
- Kiem tra chieu VIN+ va VIN-
- Neu dong lon hon 3.2A, can cau hinh lai INA219 hoac dung INA226

### 4.4. Relay khong dong/cat

**Kiem tra:**
1. Relay module co den LED sang khi GPIO HIGH?
2. Do dien ap tai chan IN cua relay (phai > 3V khi HIGH)
3. Relay module VCC = 5V? (mot so module 5V khong kich duoc bang 3.3V tu ESP32)

**Xu ly neu relay 5V khong kich duoc bang 3.3V:**
- Dung module relay co opto-isolator (IN chi can 3.3V)
- Hoac dung MOSFET/transistor lam tang de kich

### 4.5. OLED khong hien thi

**Trieu chung:** `[OLED] SSD1306 not found` hoac man hinh den

**Kiem tra:**
1. Day SDA (GPIO 21) va SCL (GPIO 22) dung chua? (chung bus voi ADS1115, INA219)
2. VCC = 3.3V (mot so module chap nhan 5V, nhung 3.3V an toan hon)
3. Thu doi dia chi: sua `OLED_ADDR` trong `oled_display.h` tu `0x3C` sang `0x3D`
4. Chay I2C scan (xem muc 4.1) de xac nhan dia chi

**Cac loi thuong gap:**

| Trieu chung                    | Cach sua                                    |
|--------------------------------|---------------------------------------------|
| Man hinh sang nhung khong co chu | Kiem tra thu vien Adafruit_SSD1306 da cai  |
| Hien thi nhieu/loan ky tu      | Kiem tra OLED_WIDTH/HEIGHT dung 128x64      |
| Chi hien splash roi den        | Kiem tra ham `oled.update()` co goi trong loop |

### 4.6. Tam pin mat troi khong sac duoc

**Trieu chung:** Module sac khong co den LED, pin khong tang ap

**Kiem tra:**
1. Do dien ap dau ra tam solar bang van nang (phai > 5V khi co anh sang)
2. Diode 1N5819 dung chieu? (Anode huong solar, Cathode huong charger)
3. Module sac co den LED bao? (TP4056: do = dang sac, xanh = day)
4. Thu dung den ban chieu truc tiep vao tam solar

**Luu y:**
- Tam 6V 1W cho dong rat nho (~170mA), sac se cham
- Trong nha kem anh sang, dien ap co the < 5V → khong du de sac
- Khi demo trong phong, dung DC Power Supply thay the neu solar yeu

---

## 5. Van de Mang

### 5.1. ESP32 khong ket noi WiFi

**Kiem tra:**
- SSID va password trong `config.h` co dau cach, ky tu dac biet?
- ESP32 chi ho tro WiFi 2.4GHz (khong ho tro 5GHz)
- Thu hotspot dien thoai voi ten WiFi don gian, khong dau

### 5.2. ESP32 ket noi WiFi nhung khong ket noi MQTT

**Kiem tra:**
- IP broker trong `config.h` co dung khong?
- May chu broker va ESP32 co cung subnet?
- Windows Firewall co chan port 1883 khong?

**Mo port tren Windows Firewall (PowerShell Admin):**

```powershell
New-NetFirewallRule -DisplayName "MQTT 1883" -Direction Inbound -Protocol TCP -LocalPort 1883 -Action Allow
```

---

## 6. Van de Docker

### 6.1. Docker build loi

```bash
# Xoa cache va build lai
docker compose build --no-cache

# Kiem tra Docker co chay khong
docker info
```

### 6.2. Container khoi dong roi dung ngay

```bash
# Xem log cua container bi loi
docker compose logs <service_name>

# Khoi dong lai
docker compose restart <service_name>
```

### 6.3. Het dung luong dia

```bash
# Don dep Docker
docker system prune -a
```

---

## 7. Kiem tra nhanh (Quick Health Check)

Chay cac lenh sau de kiem tra toan bo he thong:

```powershell
# 1. Kiem tra tat ca Docker services
docker compose ps

# 2. Test MQTT broker
mosquitto_pub -h localhost -t "test" -m "ping"

# 3. Test Backend API
curl http://localhost:8000/api/health

# 4. Test InfluxDB
curl http://localhost:8086/health

# 5. Test Frontend (phai tra ve HTML)
curl http://localhost:3000
```

Tat ca phai tra ve thanh cong. Neu service nao loi, xem muc tuong ung o tren.

---

## 8. Cach reset toan bo he thong

Neu moi thu rong loan va muon bat dau lai:

```bash
# Dung va xoa tat ca containers + volumes
docker compose down -v

# Xoa images da build
docker compose down --rmi all

# Khoi dong lai tu dau
docker compose up -d --build
```

> **Luu y:** Lenh `down -v` se xoa toan bo du lieu InfluxDB. Chi dung khi thuc su
> muon reset.

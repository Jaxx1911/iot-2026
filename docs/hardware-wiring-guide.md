# Huong dan ket noi phan cung (Hardware Wiring Guide)

## Tong quan

Tai lieu nay huong dan ket noi day giua cac module cam bien, vi dieu khien ESP32,
relay va pack pin LiFePO4 4S. Tat ca chan (pin) duoc dinh nghia trong
`firmware/src/config.h`.

---

## 1. So do chan ESP32 DevKit V1

```
                    ┌──────────────┐
              3V3  ─┤              ├─ VIN (5V)
              GND  ─┤              ├─ GND
              D15  ─┤              ├─ D13
               D2  ─┤              ├─ D12
               D4  ─┤  ESP32       ├─ D14
        (RX2) D16  ─┤  DevKit V1  ├─ D27
        (TX2) D17  ─┤              ├─ D26
               D5  ─┤              ├─ D25
              D18  ─┤              ├─ D33
              D19  ─┤              ├─ D32
         (SDA) D21 ─┤              ├─ D35 (input only)
         (SCL) D22 ─┤              ├─ D34 (input only)
              D23  ─┤              ├─ VN
                    └──────────────┘
```

### Chan su dung trong du an

| Chan ESP32 | Chuc nang         | Ket noi den                        |
|------------|-------------------|------------------------------------|
| GPIO 21    | I2C SDA           | ADS1115 SDA, INA219 SDA, OLED SDA |
| GPIO 22    | I2C SCL           | ADS1115 SCL, INA219 SCL, OLED SCL |
| GPIO 4     | OneWire Data      | DS18B20 Data                       |
| GPIO 16    | Relay Charge      | Relay module 1 IN                  |
| GPIO 17    | Relay Discharge   | Relay module 2 IN                  |
| 3V3        | Nguon logic 3.3V  | Cam bien, pull-up, OLED VCC        |
| GND        | Mass chung        | Tat ca GND                         |
| VIN (5V)   | Nguon 5V (tu USB hoac Buck) | Relay VCC            |

---

## 2. Ket noi ADS1115 (Do dien ap tung cell)

Module ADS1115 doc dien ap tung cell qua cau phan ap. ESP32 ADC chi doc 0-3.3V
nen can chia ap tu 0-3.65V xuong 0-1.22V (an toan cho ADC 16-bit).

### Ket noi I2C

```
ADS1115        ESP32
────────       ─────
VDD      ──── 3V3
GND      ──── GND
SDA      ──── GPIO 21
SCL      ──── GPIO 22
ADDR     ──── GND        (dia chi I2C = 0x48)
```

### Cau phan ap cho tung cell

Moi cell can mot mach chia ap rieng. Dung Rtop = 20kOhm, Rbot = 10kOhm.
Ti so chia: Vout = Vcell x 10/(20+10) = Vcell / 3.0

```
          Cell+ ────┬──── [20kΩ Rtop] ────┬──── [10kΩ Rbot] ────┬──── Cell-
                    │                      │                      │
                    │                   ADS1115                   │
                    │                   A0/A1/A2/A3               │
                    │                      │                      │
                    └──────────────────────┘                      │
                                                                 GND
```

**Cach do tung cell trong 4S noi tiep:**

| Kenh ADS1115 | Do dien ap         | Cach tinh                     |
|-------------|--------------------|-------------------------------|
| A0          | Cell 1 (duoi cung) | V_A0 x 3.0 = Vcell1          |
| A1          | Cell 1 + Cell 2    | (V_A1 x 3.0) - Vcell1 = Vcell2 |
| A2          | Cell 1+2+3         | (V_A2 x 3.0) - Vcell1 - Vcell2 = Vcell3 |
| A3          | Toan bo 4S         | (V_A3 x 3.0) - Vcell1 - Vcell2 - Vcell3 = Vcell4 |

> **Luu y:** Cach don gian nhat cho demo la do rieng tung cell bang cau phan ap
> doc lap (khong noi tiep stack). Neu do stack, can hieu chinh phan mem.

### Cach lam don gian (demo)

Neu chi can demo, co the tach tung cell ra do doc lap:
- A0 ← Cell 1 qua cau phan ap
- A1 ← Cell 2 qua cau phan ap
- A2 ← Cell 3 qua cau phan ap
- A3 ← Cell 4 qua cau phan ap
- GND cau phan ap = GND cua cell tuong ung

Firmware hien tai (`sensors.cpp`) doc truc tiep tung kenh va nhan voi `VDIV_RATIO = 3.0`.

---

## 3. Ket noi INA219 (Do dong dien va dien ap bus)

INA219 mac noi tiep vao duong dong chinh cua pack de do dong va dien ap pack.

### Ket noi I2C

```
INA219         ESP32
────────       ─────
VCC      ──── 3V3
GND      ──── GND
SDA      ──── GPIO 21   (chung bus I2C voi ADS1115)
SCL      ──── GPIO 22
A0       ──── GND        (dia chi I2C = 0x40, mac dinh)
A1       ──── GND
```

### Ket noi dong luc

```
Pack+ ──── [Cau chi 10A] ──── INA219 VIN+ ──── INA219 VIN- ──── Tai (hoac Relay)
```

INA219 co dien tro shunt 0.1 Ohm tren board. Do duoc dong toi da khoang ±3.2A
(che do mac dinh). Neu can do dong lon hon, thay dien tro shunt hoac dung INA226.

> **Quan trong:** Noi VIN+ va VIN- theo dung chieu dong chay. Dong duong = sac,
> dong am = xa.

---

## 4. Ket noi DS18B20 (Do nhiet do)

Dung 2 cam bien DS18B20 chong nuoc, gan vao cell nong nhat va gan relay/MOSFET.

### So do ket noi

```
DS18B20 (day do)    ──── GPIO 4 (OneWire Data)
DS18B20 (day den)   ──── GND
DS18B20 (day vang)  ──── 3V3

         3V3
          │
         [4.7kΩ]     ← Pull-up bat buoc cho bus OneWire
          │
GPIO 4 ───┴──── DS18B20 #1 Data ──── DS18B20 #2 Data
```

Nhieu cam bien DS18B20 co the mac chung mot bus OneWire (cung GPIO 4).
Firmware phan biet bang dia chi ROM tu dong.

### Vi tri dat cam bien

- **Cam bien 1:** Gan sat cell nong nhat (thuong la cell giua pack)
- **Cam bien 2:** Gan MOSFET/Relay cong suat (giam sat nhiet phan tu dong)

---

## 5. Ket noi OLED Display SSD1306 128x64

Man hinh OLED 0.96 inch I2C hien thi trang thai BMS tai cho (khong can dashboard web).
Chung bus I2C voi ADS1115 va INA219 nhung dia chi khac (0x3C).

### So do ket noi

```
OLED SSD1306       ESP32
────────────       ─────
VCC          ──── 3V3
GND          ──── GND
SDA (hoac SDA) ── GPIO 21  (chung bus I2C)
SCL (hoac SCK) ── GPIO 22  (chung bus I2C)
```

### Dia chi I2C

OLED SSD1306 thuong co dia chi mac dinh `0x3C`. Mot so module la `0x3D`.
Neu OLED khong hien thi, thu doi dia chi trong `oled_display.h`:

```cpp
#define OLED_ADDR 0x3C   // Thu 0x3D neu khong duoc
```

### Hien thi

OLED tu dong xoay 2 trang moi 4 giay:

- **Trang 1:** Vpack (lon), SOC bar, dong dien, 2 nhiet do, WiFi/MQTT status
- **Trang 2:** Dien ap 4 cell + thanh bar, trang thai Charge/Discharge relay
- **Khi co loi:** Hien thi canh bao lon voi loai loi va gia tri

> **Luu y:** OLED va ADS1115 va INA219 deu dung chung bus I2C. Moi thiet bi co
> dia chi rieng (0x3C, 0x48, 0x40) nen khong xung dot.

---

## 6. Ket noi tam pin mat troi (Solar Panel)

Tam pin mat troi mini 6V 1W/5W la nguon nang luong tai tao thuc te cho demo.

### So do ket noi

```
Solar Panel 6V ──── [Diode 1N5819] ──── Module sac TP4056/CN3791
                     (Anode → Solar)      │
                     (Cathode → Charger)  │ Output
                                          ↓
                                    Pack LiFePO4 4S
                                    (qua cau chi 10A)
```

### Chi tiet

| Thanh phan              | Vai tro                                            |
|-------------------------|----------------------------------------------------|
| Solar Panel 6V 1W       | Nguon nang luong, ~170mA max khi nang tot          |
| Solar Panel 6V 5W       | Nguon manh hon, ~830mA max (tuy chon)              |
| Diode Schottky 1N5819   | Chong dong nguoc tu pin ve solar khi troi toi       |
| TP4056 module           | Sac 1 cell LiFePO4 (demo don gian)                 |
| CN3791 module           | Sac co MPPT, hieu quat cao hon (khuyen nghi)       |

### Luu y khi dung solar

- Tam 6V 1W chi cho dong nho (~170mA), phu hop demo khai niem
- De sac pack 4S, can mach boost 6V → 14.6V hoac dung nhieu tam noi tiep
- Cach don gian nhat: demo sac 1 cell rieng bang solar, pack 4S sac bang DC Supply
- Khi demo, dat tam solar duoi den hoac gan cua so co anh sang

---

## 7. Ket noi Relay (Dong/cat sac xa)

Dung 2 relay module 5V opto-isolated. Moi relay cat mot duong:
- Relay 1: Duong sac (Charge path)
- Relay 2: Duong xa (Discharge path)

### So do ket noi

```
ESP32                  Relay Module
─────                  ────────────
GPIO 16 (CHARGE)  ──── IN1
GPIO 17 (DISCHARGE)──── IN2
5V (VIN hoac Buck) ──── VCC
GND                ──── GND

              Relay COM ────── Pack+ (hoac nguon sac)
              Relay NO  ────── Tai (hoac sac)
              Relay NC  ────── (khong dung)
```

### Logic dieu khien

| GPIO | Trang thai HIGH | Trang thai LOW |
|------|----------------|----------------|
| 16   | Relay 1 dong (sac ON) | Relay 1 mo (sac OFF) |
| 17   | Relay 2 dong (xa ON)  | Relay 2 mo (xa OFF)  |

> **An toan:** Khi mat dien hoac ESP32 reset, GPIO se ve LOW → relay mo → cat
> sac/xa. Day la trang thai an toan (fail-safe).

### Diode bao ve

Neu dung relay cuon 5V/12V thay vi module co san, them diode 1N4007 nguoc qua
cuon day relay de chong xung:

```
Relay coil+ ───┬──── VCC
               │
           [1N4007]  (Cathode huong VCC)
               │
Relay coil- ───┴──── Transistor/MOSFET
```

---

## 8. Cap nguon he thong

### Phuong an 1: Cap tu USB (khi dev)

- ESP32 cap qua USB Micro-B tu may tinh
- Relay module cap 5V tu chan VIN cua ESP32

### Phuong an 2: Cap tu pack pin (khi demo doc lap)

```
Pack 12.8V ──── [Buck LM2596] ──── 5V output ──┬── ESP32 VIN
                                                ├── Relay VCC
                                                └── (Cam bien dung 3V3 tu ESP32)
```

Chinh Buck LM2596:
1. Ket noi dau vao voi pack (12-14V)
2. Dung dong ho van nang do dau ra
3. Van biet tro tren module cho den khi dau ra = 5.0V
4. Ket noi vao ESP32 VIN

> **Canh bao:** Khong cap qua 6V vao chan VIN cua ESP32 DevKit.

---

## 9. So do tong hop ket noi toan bo

```
  [Solar 6V]──[1N5819]──[TP4056/CN3791]
                              │ sac
                         ┌────┴────────────────────────────────┐
                         │          LiFePO4 4S Pack            │
                         │  [Cell1]─[Cell2]─[Cell3]─[Cell4]    │
                         └───┬──┬──┬──┬────────────┬───────────┘
                             │  │  │  │            │
                     Cau phan ap (x4)          [Cau chi 10A]
                             │  │  │  │            │
                         ┌───┴──┴──┴──┴───┐   ┌───┴───┐
                         │   ADS1115      │   │INA219 │
                         │  A0 A1 A2 A3   │   │VIN+/- │
                         └──────┬─────────┘   └───┬───┘
                           I2C  │             I2C │
                    ┌───────────┴─────────────────┴────┐
                    │           ESP32 DevKit            │
  ┌──────────┐      │  SDA=21  SCL=22  OW=4  R1=16 R2=17│
  │OLED 0.96"│──I2C─│  OLED addr=0x3C                   │
  │ SSD1306  │      └──┬─────────┬──────────┬──────────┘
  └──────────┘         │         │          │
                  [4.7kΩ]    [Relay 1]  [Relay 2]
                       │      Charge    Discharge
                  ┌────┴────┐    │          │
                  │DS18B20  │    │          │
                  │ #1  #2  │  [Buck LM2596] ← Pack 12.8V
                  └─────────┘    │
                              5V output → ESP32 VIN + Relay VCC
```

---

## 10. Checklist truoc khi cap dien

- [ ] Kiem tra cuc tinh pack pin (do bang van nang truoc)
- [ ] Kiem tra cau chi da lap dung vi tri
- [ ] Kiem tra tat ca ket noi GND chung
- [ ] Kiem tra dien ap Buck output = 5.0V truoc khi noi vao ESP32
- [ ] Kiem tra cau phan ap: do dien ap tai dau ra, phai < 1.3V
- [ ] Kiem tra INA219 VIN+/VIN- dung chieu
- [ ] Kiem tra pull-up 4.7kOhm cho DS18B20
- [ ] Kiem tra relay module: IN noi dung GPIO, VCC = 5V
- [ ] Kiem tra OLED: 4 day I2C dung (VCC, GND, SDA, SCL)
- [ ] Kiem tra solar panel: diode 1N5819 dung chieu (Anode → Solar, Cathode → Charger)
- [ ] Khong co day nao bi chap hoac hong cach dien

> **Quy tac an toan:** Luon do kiem bang van nang TRUOC khi cap dien.
> Khi demo, dat dong gioi han thap (5A). Luon co nguoi giam sat.

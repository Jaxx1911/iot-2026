# ESP32 Battery Monitor (INA219 + DS18B20 + LCD + MQTT + Relay)

Tai lieu nay mo ta chi tiet ma nguon hien tai: cau truc file, y nghia tung function, va luong goi ham trong `setup()` / `loop()`.

## 1) Tong quan he thong

Du an doc cac dai luong:

- Dien ap va dong qua INA219
- Nhiet do qua DS18B20
- Hien thi len LCD I2C 16x2
- Dieu khien relay theo nhiet do + lenh MQTT
- Gui telemetry len MQTT broker

## 2) Cau truc file

- `sketch_apr11a.ino`
  - File entry-point cua Arduino
  - Tao object phan cung
  - Chua `setup()` va `loop()`
- `config.h`
  - Tat ca hang so cau hinh (nguong, chan, timeout, mode)
  - Include secrets tu `secrets.h` (hoac `secrets.example.h`)
- `globals.h`
  - Khai bao `extern` cho object/variable dung chung giua cac module
- `battery_calc.h/.cpp`
  - Bang map V->%
  - Moving-average dien ap
  - Slew-limit % pin
  - EMA nhiet do hien thi
- `display_ui.h/.cpp`
  - Format va render LCD
  - Xac dinh mode Charge/Dischg/Idle theo dong
- `relay_mqtt.h/.cpp`
  - Relay logic (auto/force)
  - WiFi setup/reconnect
  - MQTT setup/reconnect/publish/command callback
- `secrets.example.h`
  - Mau thong tin WiFi/MQTT de copy thanh `secrets.h`
- `secrets.h`
  - Thong tin that (khong commit len git)

## 3) Cac bien va object toan cuc

Trong `sketch_apr11a.ino`:

- Hardware objects:
  - `lcd`
  - `wifiClient`
  - `mqttClient`
  - `ina219`
  - `oneWire`
  - `sensors`
- Runtime state:
  - `% pin hien thi`: `g_percentDisp`
  - `T hien thi`: `g_tempDisp`
  - state mode hien thi: `g_modeLastSample`, `g_modeSameCount`, `g_modeDisplay`
  - relay state: `g_autoThermalCut`, `g_relayMode`, `g_relayCutApplied`
  - telemetry snapshot: `g_lastVBus`, `g_lastVAvg`, `g_lastCurrent`, `g_lastTempC`, `g_lastPercent`, `g_lastCharging`, `g_lastTempValid`

## 4) Config va y nghia thong so (`config.h`)

### 4.1 Battery/percent

- `CHARGE_I_MA_MIN`: nguong coi la dang sac
- `CHARGE_V_OFFSET`: so volt tru bo khi dang sac
- `CHARGE_BU_APPLY_ABOVE_V`: chi tru bo khi dien ap vuot nguong nay
- `HEAVY_LOAD_I_MA`: tai nang thi bo qua push MA
- `VBUS_PLAUSIBLE_LOW_V`: dien ap duoi nguong coi la khong tin cay

### 4.2 Relay/nhiet

- `TEMP_CUT_CHARGE_C`: vuot nguong thi cat relay
- `TEMP_RESTORE_CHARGE_C`: xuong duoi nguong thi cho phep noi lai (hysteresis)
- `RELAY_PIN`, `RELAY_ACTIVE_LEVEL`, `RELAY_IDLE_LEVEL`

### 4.3 UI smoothing

- `TEMP_SMOOTH_ALPHA`: he so EMA cho nhiet do
- `PERCENT_MAX_SLEW_PER_S`: gioi han toc do doi % pin

### 4.4 Network

- `WIFI_CONNECT_TIMEOUT_MS`
- `MQTT_PORT`, `MQTT_RECONNECT_MS`, `MQTT_PUBLISH_MS`
- `DEVICE_ID`: namespace topic MQTT

### 4.5 Secrets

`config.h` su dung:

1. `secrets.h` neu ton tai
2. neu khong thi fallback `secrets.example.h`

Canh bao: `secrets.h` phai duoc `.gitignore`.

## 5) Function chi tiet theo module

## 5.1 `battery_calc.cpp`

### `batteryMovingAveragePushIf(float vAdj, bool allowPush)`

- Neu `allowPush == false` thi bo qua
- Neu buffer chua primed thi fill toan bo = `vAdj`
- Neu da primed thi ghi vao vi tri `g_maIdx` va quay vong

### `batteryMovingAverageAvg()`

- Tinh trung binh MA tren buffer

### `batteryLookupPercent(float v)`

- Clamp 2 dau bang map table
- Noi suy tuyen tinh giua 2 moc dien ap lien ke

### `batteryProcessPercentSlew(float percentTarget, uint32_t loopMs)`

- Lan dau set thang `g_percentDisp`
- Sau do gioi han do thay doi theo `PERCENT_MAX_SLEW_PER_S`
- Neu tat slew (gia tri <= 0) thi fallback EMA nhe

### `batteryProcessTempEma(float tempC)`

- Khoi tao lan dau
- Sau do cap nhat theo EMA (`TEMP_SMOOTH_ALPHA`)

## 5.2 `display_ui.cpp`

### `ui_lcdPrintFloatOneDecimal(float x)`

- In so 1 chu so sau dau phay, xu ly am/duong

### `ui_modeFromCurrent(float raw_mA)`

- `< -5mA` => Dischg
- `> +5mA` => Charge
- Con lai => Idle

### `ui_modeLabel(int m)`

- Tra chuoi ten mode theo ma so

### `ui_render(...)`

- Dong 1: `% pin` + `I`
- Dong 2:
  - Neu `stopChargeWarn` => `RELAY CUT TEMP!`
  - Nguoc lai hien `mode + nhiet do`
  - Neu `tempValid == false` hien `--.-C`

## 5.3 `relay_mqtt.cpp`

### `relayModeStr(RelayMode mode)`

- Doi enum relay mode sang chuoi JSON/log

### `mqttCallback(char *topic, byte *payload, unsigned int length)` (static)

- Nhan lenh tu topic cmd
- Ho tro payload:
  - `auto`
  - `cut` / `off` / `0`
  - `connect` / `on` / `1`
- Cap nhat `g_relayMode` va publish status ack

### `relayInitPinsEarly()`

- Set chan relay output + muc idle rat som trong startup

### `relayApplyCut(bool cut)`

- Ghi relay chan theo `ACTIVE/IDLE`
- Dong bo `g_relayCutApplied`

### `relayThermalUpdate(float tempC, bool tempValid)`

- Auto mode hysteresis:
  - Tren nguong cat => `g_autoThermalCut = true`
  - Duoi nguong phuc hoi => `g_autoThermalCut = false`

### `relayUpdateOutput()`

- Hop nhat trang thai auto va force mode
- Thuc thi bang `relayApplyCut(...)`
- Tra ket qua dang cut hay khong

### `setupWifi()`

- Bat station mode
- Thu ket noi den timeout
- In IP neu thanh cong

### `setupMqtt()`

- Tao topic base theo `DEVICE_ID`
- Bat TLS insecure (`setInsecure`)
- Set server + callback
- In thong tin host/topic

### `ensureWifi()`

- Neu mat WiFi thi reconnect

### `ensureMqtt()`

- Neu mat MQTT thi reconnect theo chu ky `MQTT_RECONNECT_MS`
- Khi reconnect ok:
  - subscribe topic command
  - publish online status

### `publishTelemetry()`

- Publish JSON dinh ky theo `MQTT_PUBLISH_MS`
- Truong JSON:
  - `vbus`, `vavg`, `current_mA`, `temp_c`, `percent`
  - `charging`, `temp_valid`
  - `relay_cut`, `relay_mode`

## 6) Flow goi ham trong `setup()`

Trong `sketch_apr11a.ino`:

1. `relayInitPinsEarly()`
2. `Serial.begin(115200)`
3. `relayApplyCut(false)`
4. `Wire.begin(21,22)`
5. `ina219.begin()`
6. `ina219.setCalibration_16V_400mA()`
7. `sensors.begin()`
8. In so DS18B20 detect duoc
9. `lcd.init()` + `lcd.backlight()`
10. `setupWifi()`
11. `setupMqtt()`

## 7) Flow goi ham trong `loop()`

Chu ky loop ~ 1 giay (`loopMs = 1000`):

1. Doc cam bien:
   - `raw_mA = ina219.getCurrent_mA()`
   - `current = raw_mA * INA219_CURRENT_SIGN`
   - `vBus = ina219.getBusVoltage_V()`
   - `tempC` tu DS18B20
2. Xu ly V pin:
   - Nhan dien trang thai `charging`
   - Co the tru `CHARGE_V_OFFSET` khi dang sac va dien ap cao
   - Chan mau outlier (tai nang / vbus implausible)
   - Push MA + tinh `avgV`
3. Doi `avgV` -> `percentTarget` bang table
4. Xu ly relay:
   - `relayThermalUpdate(tempC, tempValid)`
   - `stopChargeWarn = relayUpdateOutput()`
5. Lam muot:
   - `%`: `batteryProcessPercentSlew(...)`
   - `T`: `batteryProcessTempEma(...)` (chi khi tempValid)
6. Xac dinh mode hien thi:
   - `modeNow = ui_modeFromCurrent(raw_mA)`
   - Debounce mode qua `g_modeSameCount`
7. Cap nhat snapshot telemetry (`g_last...`)
8. In Serial status dong tong hop
9. Mang:
   - `ensureWifi()`
   - `ensureMqtt()`
   - `mqttClient.loop()`
   - `publishTelemetry()`
10. Hien thi LCD:
    - `ui_render(...)`
11. `delay(loopMs)`

## 8) MQTT contract

### Topics

- Base: `iot/<DEVICE_ID>`
- Telemetry: `iot/<DEVICE_ID>/telemetry`
- Status: `iot/<DEVICE_ID>/status`
- Command: `iot/<DEVICE_ID>/relay/cmd`

### Command payload hop le

- `auto`
- `cut` | `off` | `0`
- `connect` | `on` | `1`

## 9) Cac tinh huong thuong gap

- `T=-127` hoac `So cam bien: 0`:
  - DS18B20 chua detect duoc (wiring/pull-up/GPIO sai)
- `Vbus<2.5` lien tuc:
  - Nhanh do INA219 dang sai tham chieu GND/VIN
- `I ~ 0` du tai chay:
  - Dong dang bypass shunt, khong di qua VIN- -> VIN+
- Relay dao logic:
  - Dao `RELAY_ACTIVE_LEVEL` / `RELAY_IDLE_LEVEL`

## 10) Huong dan maintain

- Uu tien sua nguong trong `config.h`
- Khong hard-code mat khau vao code
- Khi doi logic module, cap nhat prototype trong `.h` truoc, sau do cap nhat `.cpp`


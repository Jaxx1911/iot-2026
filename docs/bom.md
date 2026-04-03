# BOM (Bill of Materials) – BMS LiFePO4 4S Demo

## 1. Khối pin


| #   | Tên thiết bị                    | Số lượng | Ghi chú                           |
| --- | ------------------------------- | -------- | --------------------------------- |
| 1   | Cell LiFePO4 32700 3.2V 6000mAh | 4        | Ghép 4S, mua cùng loại cùng batch |
| 2   | Holder pin 32700 (1 slot)       | 4        | Hoặc 1 holder 4-slot              |
| 3   | Thanh nickel / dây nối cell     | 4        | Nối tiếp 4S                       |
| 4   | Cầu chì DC 10A + holder         | 1        | Bảo vệ ngắn mạch                  |


## 2. Vi điều khiển & truyền thông


| #   | Tên thiết bị             | Số lượng | Ghi chú                      |
| --- | ------------------------ | -------- | ---------------------------- |
| 5   | ESP32 DevKit V1 (CP2102) | 1        | WiFi + BLE tích hợp          |
| 6   | Cable USB Micro-B        | 1        | Nạp firmware + cấp nguồn dev |


## 3. Cảm biến đo lường & hiển thị


| #   | Tên thiết bị                          | Số lượng | Ghi chú                                    |
| --- | ------------------------------------- | -------- | ------------------------------------------ |
| 7   | Module ADS1115 16-bit ADC (I2C)       | 1        | 4 kênh, đọc Vcell qua cầu phân áp          |
| 8   | Module INA219 current sensor (I2C)    | 1        | Đo dòng + điện áp bus pack                 |
| 9   | Cảm biến nhiệt DS18B20 waterproof     | 2        | 1-Wire, gắn lên cell nóng nhất             |
| 10  | Điện trở 4.7kΩ (pull-up 1-Wire)       | 1        | Pull-up cho bus DS18B20                    |
| 11  | Điện trở cầu phân áp (10kΩ + 20kΩ)    | 4 bộ     | Chia áp cell xuống 0-3.3V cho ADC          |
| 12  | OLED Display 0.96" I2C SSD1306 128x64 | 1        | Hiển thị trạng thái tại chỗ, I2C addr 0x3C |


## 4. Chấp hành / công suất


| #   | Tên thiết bị                       | Số lượng | Ghi chú                       |
| --- | ---------------------------------- | -------- | ----------------------------- |
| 13  | Relay module 5V 1-channel opto     | 2        | 1 charge, 1 discharge path    |
| 14  | MOSFET IRF540N + gate driver (opt) | 1        | Dự phòng nếu muốn PWM control |
| 15  | Diode 1N4007 (flyback)             | 2        | Bảo vệ relay coil             |


## 5. Nguồn năng lượng tái tạo


| #   | Tên thiết bị                                | Số lượng | Ghi chú                                       |
| --- | ------------------------------------------- | -------- | --------------------------------------------- |
| 16  | Tấm pin mặt trời mini 6V 1W polycrystalline | 1        | Nguồn sạc solar thật cho demo                 |
| 17  | Module sạc TP4056 / CN3791 (MPPT mini)      | 1        | Sạc LiFePO4 từ solar, tùy chọn CN3791 có MPPT |
| 18  | Diode Schottky 1N5819                       | 1        | Chống dòng ngược từ pin về solar panel        |


> **Ghi chú:** Tấm pin 6V 1W cho dòng ~~170mA ở điều kiện nắng tốt. Đủ để demo
> sạc 1 cell LiFePO4 hoặc sạc chậm pack 4S qua mạch boost. Nếu muốn sạc nhanh
> hơn, dùng tấm 6V 5W (~~830mA).

## 6. Phụ kiện & dụng cụ


| #   | Tên thiết bị                     | Số lượng | Ghi chú                         |
| --- | -------------------------------- | -------- | ------------------------------- |
| 19  | DC Power Supply 0-30V 5A         | 1        | Nguồn lab dự phòng / test       |
| 20  | Module Buck LM2596 DC-DC         | 1        | Hạ áp pack 12.8V → 5V cấp ESP32 |
| 21  | Breadboard 830 điểm              | 1        |                                 |
| 22  | Dây cắm breadboard (bộ M-M, M-F) | 1 bộ     |                                 |
| 23  | Terminal block 2P (vít)          | 4        | Kết nối dây công suất           |
| 24  | Dây silicon 18AWG (đỏ + đen)     | 1m mỗi   | Dây tải dòng                    |
| 25  | Đồng hồ vạn năng                 | 1        | Đo kiểm tra, hiệu chuẩn         |


## 7. Gateway / Server (tùy chọn)


| #   | Tên thiết bị               | Số lượng | Ghi chú                                            |
| --- | -------------------------- | -------- | -------------------------------------------------- |
| 26  | Raspberry Pi 4 Model B 4GB | 0-1      | Chạy Docker (MQTT+BE+DB). Dùng laptop nếu không có |


## Tên tra cứu mua hàng (Shopee / Lazada / Điện Tử Việt)

- `Cell pin LiFePO4 32700 3.2V 6000mAh`
- `ESP32 DevKit V1` hoặc `ESP32 WROOM 32`
- `Module ADS1115 ADC 16bit I2C`
- `Module INA219 cảm biến dòng điện`
- `Cảm biến nhiệt độ DS18B20 chống nước`
- `OLED 0.96 inch I2C SSD1306 128x64`
- `Tấm pin mặt trời mini 6V 1W polycrystalline` hoặc `Solar Panel 6V 5W mini`
- `Module sạc solar TP4056` hoặc `CN3791 MPPT solar charger`
- `Diode Schottky 1N5819`
- `Relay module 5V 1 kênh opto`
- `Module buck LM2596 DC-DC`
- `Nguồn DC điều chỉnh 30V 5A` hoặc `DC Power Supply 30V 5A`
- `Breadboard 830 điểm`
- `Cầu chì DC 10A`
- `Điện trở 10K 20K 4.7K 1/4W`

## Ước tính chi phí (VNĐ)


| Hạng mục                 | Ước tính     |
| ------------------------ | ------------ |
| 4x Cell LiFePO4          | 280,000      |
| ESP32 DevKit             | 85,000       |
| ADS1115 + INA219         | 80,000       |
| DS18B20 x2               | 30,000       |
| OLED SSD1306 0.96"       | 50,000       |
| Solar Panel 6V 1W        | 45,000       |
| Module sạc TP4056/CN3791 | 25,000       |
| Relay + MOSFET           | 40,000       |
| Nguồn DC 30V 5A          | 250,000      |
| Buck LM2596              | 15,000       |
| Breadboard + dây         | 50,000       |
| Linh kiện rời            | 30,000       |
| **Tổng ước tính**        | **~980,000** |



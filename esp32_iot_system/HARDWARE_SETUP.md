# 🔧 Hướng dẫn lắp đặt phần cứng ESP32-S3 IoT System

## 📋 Danh sách linh kiện

### Board chính:
- ✅ ESP32-S3 Development Board (BEVA hoặc tương tự)
- ✅ Màn hình LCD tích hợp
- ✅ PIR sensor tích hợp  
- ✅ 2x Relay outputs tích hợp
- ✅ 4x Push buttons (BACK, NEXT, OK, CANCEL)

### Cảm biến ngoài:
- 🔄 **AHT22** - Cảm biến nhiệt độ và độ ẩm
- 🔄 **BMP280** - Cảm biến nhiệt độ và áp suất khí quyển

## 🔌 Sơ đồ kết nối I2C

### Pin I2C trên ESP32-S3:
```
SDA (Data)  → GPIO 21
SCL (Clock) → GPIO 22
VCC         → 3.3V  
GND         → GND
```

### Kết nối AHT22:
```
AHT22    →    ESP32-S3
VCC      →    3.3V
GND      →    GND
SDA      →    GPIO 21
SCL      →    GPIO 22
```

### Kết nối BMP280:
```
BMP280   →    ESP32-S3
VIN      →    3.3V
GND      →    GND
SDA      →    GPIO 21
SCL      →    GPIO 22
```

## 🔍 Địa chỉ I2C

- **AHT22**: `0x38` (Hex) / `56` (Dec)
- **BMP280**: `0x76` (Hex) / `118` (Dec)

## 📐 Sơ đồ kết nối vật lý

```
                    ESP32-S3 Board
    ┌─────────────────────────────────────────┐
    │  ┌─────┐                    ┌────────┐  │
    │  │ LCD │                    │  PIR   │  │
    │  └─────┘                    └────────┘  │
    │                                         │
    │  [BACK] [NEXT] [OK] [CANCEL]           │
    │                                         │
    │  GPIO21 ●━━━━━━━━━━━━━━━━━━━━━━━━━━━●    │
    │  GPIO22 ●━━━━━━━━━━━━━━━━━━━━━━━━━━━●    │
    │  3.3V   ●━━━━━━━━━━━━━━━━━━━━━━━━━━━●    │
    │  GND    ●━━━━━━━━━━━━━━━━━━━━━━━━━━━●    │
    │                                         │
    │  Terminal Blocks: [REL1] [REL2]        │
    └─────────────────────────────────────────┘
                          │
                          │ (I2C Bus)
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
    ┌───▼───┐         ┌───▼───┐         
    │ AHT22 │         │BMP280 │
    │ Temp/ │         │Temp/  │
    │ Humid │         │Press  │
    └───────┘         └───────┘
```

## ⚡ Các bước lắp đặt

### Bước 1: Chuẩn bị breadboard
1. Đặt ESP32-S3 board ở trung tâm breadboard
2. Cắm các cảm biến AHT22 và BMP280 vào breadboard

### Bước 2: Kết nối nguồn
```bash
# Kết nối 3.3V và GND
ESP32-S3 3.3V → Rail dương (+) của breadboard
ESP32-S3 GND  → Rail âm (-) của breadboard

AHT22 VCC     → Rail dương (+)
AHT22 GND     → Rail âm (-)

BMP280 VIN    → Rail dương (+)
BMP280 GND    → Rail âm (-)
```

### Bước 3: Kết nối I2C Bus
```bash
# Kết nối SDA (Data line)
ESP32-S3 GPIO21 → AHT22 SDA → BMP280 SDA

# Kết nối SCL (Clock line)  
ESP32-S3 GPIO22 → AHT22 SCL → BMP280 SCL
```

### Bước 4: Kiểm tra kết nối
```bash
# Sử dụng multimeter kiểm tra:
- Điện áp 3.3V tại VCC của các cảm biến
- Continuity test cho các đường SDA/SCL
- Không có short circuit giữa VCC và GND
```

## 🧪 Test kết nối

### Scan I2C devices:
```c
// Code test trong ESP32
i2c_cmd_handle_t cmd = i2c_cmd_link_create();
for (uint8_t addr = 1; addr < 127; addr++) {
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    if (i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50) == ESP_OK) {
        printf("Found device at 0x%02X\n", addr);
    }
}
```

**Kết quả mong đợi:**
```
Found device at 0x38  (AHT22)
Found device at 0x76  (BMP280)
```

## 🔧 Troubleshooting

### ❌ Không tìm thấy device:
- ✅ Kiểm tra lại kết nối VCC, GND
- ✅ Kiểm tra SDA/SCL không bị đảo ngược
- ✅ Thử giảm tần số I2C xuống 50kHz: `I2C_MASTER_FREQ_HZ = 50000`

### ❌ Đọc dữ liệu lỗi:
- ✅ Thêm pull-up resistor 4.7kΩ cho SDA/SCL (nếu cần)
- ✅ Kiểm tra chất lượng dây nối (không quá dài >20cm)
- ✅ Đảm bảo nguồn 3.3V ổn định

### ❌ Dữ liệu không chính xác:
- ✅ Chờ đủ thời gian cho sensor khởi động (100ms)
- ✅ Đảm bảo calibration data được đọc đúng (BMP280)
- ✅ Kiểm tra công thức tính toán trong code

## 📊 Monitoring

Sau khi lắp đặt xong, bạn có thể:

1. **Truy cập Web Interface:**
   - Kết nối WiFi với ESP32
   - Mở browser tới IP của ESP32
   - Xem dữ liệu real-time

2. **Serial Monitor:**
   ```
   AHT22 - Temperature: 25.30°C, Humidity: 65.20%
   BMP280 - Temperature: 25.10°C, Pressure: 1013.25 Pa
   Sensor Data - Temp: 25.3°C, Humidity: 65.2%, Pressure: 1013.3 hPa
   ```

3. **Điều khiển Relay:**
   - Manual mode: Bật/tắt thủ công qua web
   - Auto mode: Tự động theo ngưỡng nhiệt độ

## 🎯 Kết quả cuối cùng

Sau khi hoàn thành:
- ✅ ESP32-S3 đọc được nhiệt độ/độ ẩm từ AHT22
- ✅ ESP32-S3 đọc được nhiệt độ/áp suất từ BMP280  
- ✅ Web interface hiển thị dữ liệu real-time
- ✅ Có thể điều khiển relay manual/auto
- ✅ Lưu trữ cấu hình vào NVS 
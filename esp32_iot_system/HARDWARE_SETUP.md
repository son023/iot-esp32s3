# 🔧 Hướng dẫn lắp đặt phần cứng ESP32-S3 IoT System

## 📋 Danh sách linh kiện

### Board chính:
- ✅ ESP32-S3 Development Board (BEVA hoặc tương tự)
- ✅ 2x Relay outputs tích hợp

### Cảm biến ngoài:
- 🔄 **AHT22** - Cảm biến nhiệt độ và độ ẩm
- 🔄 **BMP180** - Cảm biến nhiệt độ và áp suất khí quyển

## 🔌 Sơ đồ kết nối I2C


### Kết nối AHT22:
```
AHT22    →    ESP32-S3
VCC      →    3.3V
GND      →    GND
SDA      →    GPIO 01
SCL      →    GPIO 02
```

### Kết nối BMP280:
```
BMP180   →    ESP32-S3
VIN      →    3.3V
GND      →    GND
SDA      →    GPIO 01
SCL      →    GPIO 02
```
# 🚀 Quick Start Guide - ESP32-S3 IoT System

## 📋 Bước 1: Chuẩn bị phần cứng

### Kết nối cảm biến:
```
AHT22:     VCC→3.3V, GND→GND, SDA→GPIO21, SCL→GPIO22
BMP280:    VIN→3.3V, GND→GND, SDA→GPIO21, SCL→GPIO22
```

### Kiểm tra kết nối:
- ✅ Đảm bảo không short circuit VCC-GND
- ✅ Kiểm tra điện áp 3.3V tại cảm biến
- ✅ Dây kết nối không quá dài (< 20cm)

## 🔧 Bước 2: Build và Flash

### Cài đặt ESP-IDF:
```bash
# Cài đặt ESP-IDF nếu chưa có
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
source export.sh
```

### Build project:
```bash
cd esp32_iot_system
idf.py set-target esp32s3
idf.py build
```

### Flash code:
```bash
# Kết nối ESP32-S3 qua USB
idf.py -p COM3 flash monitor  # Windows
idf.py -p /dev/ttyUSB0 flash monitor  # Linux
```

## 📊 Bước 3: Kiểm tra hoạt động

### Serial Monitor sẽ hiển thị:
```
I (123) MAIN: Starting ESP32 IoT System
I (234) I2C: I2C initialized successfully  
I (345) SENSORS: AHT22 initialized successfully
I (456) SENSORS: BMP280 calibration data loaded
I (567) SENSORS: BMP280 initialized successfully
I (678) WIFI: WiFi connected, IP: 192.168.1.100
I (789) WEB_SERVER: Web server started on port 80
I (890) MAIN: System initialized successfully
```

### Test I2C devices:
```
I (1000) I2C_SCANNER: Found AHT22 at address 0x38
I (1100) I2C_SCANNER: Found BMP280 at address 0x76
I (1200) SENSORS: AHT22 - Temperature: 25.30°C, Humidity: 65.20%
I (1300) SENSORS: BMP280 - Temperature: 25.10°C, Pressure: 1013.25 Pa
```

## 🌐 Bước 4: Cấu hình WiFi

### Cách 1: SmartConfig (khuyến nghị)
1. Tải app **ESP Touch** trên điện thoại
2. Kết nối điện thoại với WiFi cần kết nối
3. Mở app ESP Touch, nhập password WiFi
4. Bấm "Confirm" để gửi thông tin
5. ESP32 sẽ tự động kết nối WiFi

### Cách 2: AP Mode
1. ESP32 tạo Access Point: `ESP32_Config`
2. Kết nối điện thoại/laptop với AP này
3. Mở browser: `http://192.168.4.1`
4. Nhập thông tin WiFi và save

## 📱 Bước 5: Truy cập Web Interface

### Tìm IP của ESP32:
```bash
# Xem trong serial monitor
I (5000) WIFI: WiFi connected, IP: 192.168.1.100

# Hoặc scan network
nmap -sn 192.168.1.0/24 | grep ESP32
```

### Mở browser:
```
http://192.168.1.100
```

### Giao diện web hiển thị:
- 🌡️ **Nhiệt độ**: 25.3°C
- 💧 **Độ ẩm**: 65.2%  
- 📊 **Áp suất**: 1013.3 hPa
- 🔌 **Relay**: ON/OFF buttons
- ⚙️ **Cấu hình**: Auto/Manual mode

## 🔧 Bước 6: Cấu hình hệ thống

### Manual Mode:
- Bật/tắt relay thủ công qua web
- Suitable cho testing

### Auto Mode:
1. Chọn "Auto Mode" trên web
2. Đặt ngưỡng nhiệt độ:
   - High threshold: 30°C
   - Low threshold: 20°C
3. Hệ thống tự động:
   - Bật relay khi temp > 30°C
   - Tắt relay khi temp < 20°C

## 🔍 Troubleshooting

### ❌ Không đọc được cảm biến:
```bash
# Kiểm tra I2C scan
I (1000) I2C_SCANNER: Starting I2C device scan...
I (1100) I2C_SCANNER: No I2C devices found!
```
**Giải pháp:**
- Kiểm tra lại kết nối SDA/SCL
- Thử giảm tần số I2C: 50kHz
- Thêm pull-up resistor 4.7kΩ

### ❌ Không kết nối WiFi:
```bash
I (10000) WIFI: WiFi connection failed
```
**Giải pháp:**
- Kiểm tra SSID/Password
- Thử SmartConfig app
- Reset WiFi config trong NVS

### ❌ Web không load:
```bash
I (5000) WEB_SERVER: HTTP server failed to start
```
**Giải pháp:**
- Kiểm tra IP ESP32 trong serial
- Thử ping từ PC
- Restart ESP32

## 📈 Monitoring

### Real-time data:
- Web interface tự động refresh mỗi 5 giây
- Serial monitor hiển thị log liên tục
- Relay state được lưu vào NVS

### Data format (API):
```json
{
  "temperature": 25.3,
  "humidity": 65.2,
  "pressure": 1013.3,
  "timestamp": 1234567890
}
```

## 🎯 Kết quả mong đợi

Sau khi hoàn thành, bạn sẽ có:
- ✅ Hệ thống IoT hoàn chỉnh với ESP32-S3
- ✅ Đọc dữ liệu từ AHT22 và BMP280
- ✅ Web interface responsive, user-friendly
- ✅ Điều khiển relay manual/auto
- ✅ Lưu trữ config bền vững
- ✅ WiFi connection với SmartConfig
- ✅ Real-time monitoring 
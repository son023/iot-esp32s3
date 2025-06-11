# 🌡️ ESP32-S3 IoT System with AHT22 & BMP280

Modern IoT monitoring system with ESP32-S3, environmental sensors, and web interface.

## ✨ Features

- 🌡️ **Temperature & Humidity** - AHT22 sensor monitoring
- 📊 **Pressure Sensing** - BMP280 barometric pressure measurement  
- 🔌 **Relay Control** - Manual/Auto relay operation
- 🌐 **Web Interface** - Real-time monitoring dashboard
- 📱 **WiFi Management** - SmartConfig and AP mode setup
- 💾 **Data Storage** - NVS persistent configuration
- 🔄 **Real-time Updates** - Live sensor data display

## 🏗️ Tech Stack

**Hardware:**
- ESP32-S3 Development Board
- AHT22 Temperature/Humidity Sensor
- BMP280 Temperature/Pressure Sensor
- Built-in LCD, PIR, Relays, Buttons

**Software:**
- ESP-IDF Framework
- FreeRTOS for multitasking
- HTTP Web Server
- I2C Communication Protocol
- JSON data exchange
- NVS Flash Storage

## 🚀 Quick Start

### Prerequisites
- ESP-IDF 5.0+
- ESP32-S3 development board
- AHT22 and BMP280 sensors
- Basic electronics (breadboard, jumper wires)

### Hardware Setup
```bash
# I2C Connections
AHT22:  VCC→3.3V, GND→GND, SDA→GPIO21, SCL→GPIO22
BMP280: VIN→3.3V, GND→GND, SDA→GPIO21, SCL→GPIO22
```

### Software Installation

1. **Clone the repository**
```bash
git clone <your-repo-url>
cd esp32_iot_system
```

2. **Build and flash**
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COM3 flash monitor  # Adjust port as needed
```

3. **WiFi Configuration**
   - Method 1: Use ESP Touch app for SmartConfig
   - Method 2: Connect to ESP32_Config AP and configure via web

4. **Access Web Interface**
```bash
# Check IP in serial monitor
http://192.168.1.100  # Replace with actual IP
```

## 📁 Project Structure

```
esp32_iot_system/
├── main/                    # ESP32 source code
│   ├── main.c              # Main application entry
│   ├── sensors.c/.h        # AHT22/BMP280 sensor drivers
│   ├── wifi_manager.c/.h   # WiFi connection management
│   ├── web_server.c/.h     # HTTP server and API endpoints
│   ├── relay_control.c/.h  # Relay control logic
│   ├── nvs_storage.c/.h    # Configuration storage
│   ├── i2c_scanner.c/.h    # I2C debugging utilities
│   ├── web/                # Web interface files
│   │   ├── index.html      # Main dashboard page
│   │   ├── style.css       # Responsive CSS styling
│   │   └── script.js       # JavaScript for real-time updates
│   └── CMakeLists.txt      # Build configuration
│
├── HARDWARE_SETUP.md       # Detailed hardware connection guide
├── QUICK_START.md          # Step-by-step setup instructions
├── sdkconfig.defaults      # ESP-IDF default configuration
└── README.md               # This file
```

## 🔧 Hardware Connections

### I2C Bus Configuration:
```
ESP32-S3 Pin    Function        Sensor Connection
GPIO 21         SDA (Data)      AHT22 SDA, BMP280 SDA
GPIO 22         SCL (Clock)     AHT22 SCL, BMP280 SCL
3.3V            Power           AHT22 VCC, BMP280 VIN
GND             Ground          AHT22 GND, BMP280 GND
```

### I2C Addresses:
- **AHT22**: `0x38` (Temperature/Humidity)
- **BMP280**: `0x76` (Temperature/Pressure)

## 📊 API Endpoints

```http
GET  /                      # Main web interface
GET  /style.css             # CSS stylesheet
GET  /script.js             # JavaScript application

GET  /api/sensors           # Get sensor readings
GET  /api/relay             # Get relay status
POST /api/relay             # Control relay state/mode
POST /api/wifi              # Configure WiFi settings
POST /api/thresholds        # Set temperature thresholds
```

### Example API Response:
```json
{
  "temperature": 25.3,
  "humidity": 65.2,
  "pressure": 1013.3,
  "timestamp": 1234567890
}
```

## 🎛️ Web Interface Features

- **Real-time Dashboard**: Live sensor data with auto-refresh
- **Relay Control**: Manual ON/OFF switches
- **Auto Mode**: Temperature-based relay automation
- **Threshold Settings**: Configurable high/low temperature limits
- **WiFi Management**: Network configuration interface
- **Responsive Design**: Works on desktop and mobile

## 🔧 Configuration Options

### Sensor Settings:
- I2C frequency: 100kHz (configurable)
- Update interval: 5 seconds
- Sensor timeout: 1 second

### Relay Automation:
- Manual mode: Direct user control
- Auto mode: Temperature threshold-based
- Default thresholds: 20°C (low), 30°C (high)

### WiFi Options:
- SmartConfig for easy setup
- AP mode for manual configuration
- Persistent storage of credentials

## 🐛 Troubleshooting

**Sensor Issues:**
- Check I2C connections (SDA/SCL)
- Verify 3.3V power supply
- Use I2C scanner to detect devices
- Add pull-up resistors if needed (4.7kΩ)

**WiFi Problems:**
- Reset WiFi credentials in NVS
- Try SmartConfig with ESP Touch app
- Check router compatibility (2.4GHz only)

**Web Interface:**
- Verify ESP32 IP address in serial monitor
- Check firewall settings
- Ensure proper WiFi connection

## 📈 Performance

- **Sensor accuracy**: ±0.3°C (temperature), ±2% (humidity), ±1hPa (pressure)
- **Update rate**: 5-second intervals
- **Memory usage**: ~200KB RAM, ~1MB Flash
- **Power consumption**: ~80mA active, ~10mA deep sleep

## 🎯 Future Enhancements

- [ ] Data logging to SD card or cloud
- [ ] Mobile app with push notifications
- [ ] Multiple sensor node support
- [ ] MQTT integration for IoT platforms
- [ ] Deep sleep mode for battery operation
- [ ] OTA (Over-The-Air) firmware updates

## 📝 License

MIT License - see LICENSE file for details.

---

**Happy IoT Building! 🚀** 
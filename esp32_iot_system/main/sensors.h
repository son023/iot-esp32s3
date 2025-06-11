#ifndef SENSORS_H
#define SENSORS_H

#include "esp_err.h"

// Pin definitions (theo board ESP32-S3 thực tế)
#define I2C_MASTER_SCL_IO           2     // SCL pin (IO2)
#define I2C_MASTER_SDA_IO           1     // SDA pin (IO1)
#define I2C_MASTER_NUM              0     // I2C port number
#define I2C_MASTER_FREQ_HZ          50000  // Giảm từ 100kHz xuống 50kHz

// AHT20 I2C address (same as AHT22)
#define AHT20_ADDR                  0x38
// BMP180 I2C address
#define BMP180_ADDR                 0x77
#define BMP180_ADDR_ALT             0x76

// BMP180 Calibration data structure (khác hoàn toàn so với BMP280)
typedef struct {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
} bmp180_calib_data_t;

typedef struct {
    float temperature;
    float humidity;
} aht22_data_t;

typedef struct {
    float temperature;
    float pressure;
} bmp180_data_t;

// Cấu trúc dữ liệu tổng hợp cho tất cả sensors
typedef struct {
    // AHT22 data
    float aht22_temperature;
    float aht22_humidity;
    bool aht22_available;
    
    // BMP180 data  
    float bmp180_temperature;
    float bmp180_pressure;
    bool bmp180_available;
    
    uint32_t timestamp;
} sensor_data_t;

esp_err_t sensors_init(void);
esp_err_t read_aht22(aht22_data_t *data);
esp_err_t read_bmp180(bmp180_data_t *data);
esp_err_t get_sensor_data(sensor_data_t *data);

#endif 
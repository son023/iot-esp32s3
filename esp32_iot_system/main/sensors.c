#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sensors.h"
#include "relay_control.h"
#include "nvs_storage.h"

static const char *TAG = "SENSORS";
static bmp180_calib_data_t bmp180_calib;

esp_err_t sensors_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed");
        return ret;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed");
        return ret;
    }

    ESP_LOGI(TAG, "I2C initialized successfully");

    // Initialize AHT20
    vTaskDelay(pdMS_TO_TICKS(500));
    uint8_t aht20_init_cmd[] = {0xAC, 0x33, 0x00};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, aht20_init_cmd, sizeof(aht20_init_cmd), true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "AHT20 initialized successfully");
    } else {
        ESP_LOGE(TAG, "AHT20 initialization failed");
    }

    // Initialize BMP180
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // Read chip ID first (should be 0x55 for BMP180)
    uint8_t chip_id;
    uint8_t reg_addr = 0xD0; // Chip ID register
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &chip_id, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        if (chip_id == 0x55) {
            ESP_LOGI(TAG, "✅ BMP180 sensor detected!");
        } else {
            ESP_LOGE(TAG, "❌ Expected BMP180 (0x55), got 0x%02X", chip_id);
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Failed to read BMP180 chip ID");
        return ret;
    }
    
    // Read BMP180 calibration coefficients (0xAA to 0xBF, 22 bytes)
    uint8_t calib_data[22];
    reg_addr = 0xAA; // Start of calibration data
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, calib_data, 22, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        // Parse BMP180 calibration data (Big Endian format)
        bmp180_calib.ac1 = (calib_data[0] << 8) | calib_data[1];
        bmp180_calib.ac2 = (calib_data[2] << 8) | calib_data[3];
        bmp180_calib.ac3 = (calib_data[4] << 8) | calib_data[5];
        bmp180_calib.ac4 = (calib_data[6] << 8) | calib_data[7];
        bmp180_calib.ac5 = (calib_data[8] << 8) | calib_data[9];
        bmp180_calib.ac6 = (calib_data[10] << 8) | calib_data[11];
        bmp180_calib.b1 = (calib_data[12] << 8) | calib_data[13];
        bmp180_calib.b2 = (calib_data[14] << 8) | calib_data[15];
        bmp180_calib.mb = (calib_data[16] << 8) | calib_data[17];
        bmp180_calib.mc = (calib_data[18] << 8) | calib_data[19];
        bmp180_calib.md = (calib_data[20] << 8) | calib_data[21];
        
        ESP_LOGI(TAG, "BMP180 calibration coefficients loaded");
        
        // Validate calibration data
        if (bmp180_calib.ac1 == 0 && bmp180_calib.ac2 == 0 && bmp180_calib.ac3 == 0) {
            ESP_LOGE(TAG, "❌ BMP180 calibration data invalid! All zeros detected.");
            return ESP_FAIL;
        }
        
        ESP_LOGI(TAG, "✅ BMP180 initialized successfully");
    } else {
        ESP_LOGE(TAG, "❌ Failed to read BMP180 calibration data");
        return ret;
    }

    return ESP_OK;
}

esp_err_t read_aht22(aht22_data_t *data)
{
    uint8_t trigger_cmd[] = {0xAC, 0x33, 0x00};
    uint8_t read_data[6];

    // Send trigger command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, trigger_cmd, sizeof(trigger_cmd), true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT20 trigger command failed");
        return ret;
    }

    // Wait for measurement (AHT20 needs at least 75ms)
    vTaskDelay(pdMS_TO_TICKS(100));

    // Read data
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, read_data, sizeof(read_data), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT20 read data failed");
        return ret;
    }

    // Parse data
    uint32_t humidity_raw = ((uint32_t)read_data[1] << 12) | ((uint32_t)read_data[2] << 4) | (read_data[3] >> 4);
    uint32_t temperature_raw = ((uint32_t)(read_data[3] & 0x0F) << 16) | ((uint32_t)read_data[4] << 8) | read_data[5];

    data->humidity = (float)humidity_raw / 1048576.0 * 100.0;
    data->temperature = (float)temperature_raw / 1048576.0 * 200.0 - 50.0;

    return ESP_OK;
}

esp_err_t read_bmp180(bmp180_data_t *data)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd;
    
    // Step 1: Read uncompensated temperature (UT)
    // Write temperature measurement command (0x2E) to control register (0xF4)
    uint8_t temp_cmd[] = {0xF4, 0x2E};
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, temp_cmd, sizeof(temp_cmd), true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BMP180 temperature command failed");
        return ret;
    }
    
    // Wait for temperature conversion (at least 4.5ms)
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Read temperature data from registers 0xF6 and 0xF7
    uint8_t temp_data[2];
    uint8_t reg_addr = 0xF6;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, temp_data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BMP180 temperature read failed");
        return ret;
    }
    
    int32_t ut = (temp_data[0] << 8) | temp_data[1];
    
    // Step 2: Read uncompensated pressure (UP)
    // Write pressure measurement command (0x34 + (OSS << 6), OSS=0 for simplicity)
    uint8_t press_cmd[] = {0xF4, 0x34}; // OSS = 0 (ultra low power)
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, press_cmd, sizeof(press_cmd), true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BMP180 pressure command failed");
        return ret;
    }
    
    // Wait for pressure conversion (at least 4.5ms for OSS=0)
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Read pressure data from registers 0xF6, 0xF7, 0xF8
    uint8_t press_data[3];
    reg_addr = 0xF6;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, press_data, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BMP180 pressure read failed");
        return ret;
    }
    
    int32_t up = ((press_data[0] << 16) | (press_data[1] << 8) | press_data[2]) >> 8; // OSS = 0
    
    // Step 3: Calculate true temperature
    int32_t x1 = ((ut - bmp180_calib.ac6) * bmp180_calib.ac5) >> 15;
    int32_t x2 = (bmp180_calib.mc << 11) / (x1 + bmp180_calib.md);
    int32_t b5 = x1 + x2;
    int32_t t = (b5 + 8) >> 4; // Temperature in 0.1°C
    
    data->temperature = t / 10.0; // Convert to °C
    
    // Step 4: Calculate true pressure
    int32_t b6 = b5 - 4000;
    x1 = (bmp180_calib.b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = (bmp180_calib.ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((bmp180_calib.ac1 * 4 + x3) << 0) + 2) >> 2; // OSS = 0
    x1 = (bmp180_calib.ac3 * b6) >> 13;
    x2 = (bmp180_calib.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (bmp180_calib.ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)up - b3) * (50000 >> 0); // OSS = 0
    
    int32_t p;
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }
    
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4); // Pressure in Pa
    
    data->pressure = p / 100.0; // Convert to hPa
    
    // BMP180 reading successful
    
    return ESP_OK;
}

esp_err_t get_sensor_data(sensor_data_t *data)
{
    aht22_data_t aht_data;
    bmp180_data_t bmp_data;
    
    // Set default values
    data->temperature = 25.0;
    data->humidity = 50.0;
    data->pressure = 1013.2;
    data->timestamp = esp_log_timestamp();
    
    // Try to read AHT20
    esp_err_t aht_ret = read_aht22(&aht_data);
    if (aht_ret == ESP_OK) {
        data->temperature = aht_data.temperature;
        data->humidity = aht_data.humidity;
        // AHT20 reading successful
    } else {
        ESP_LOGE(TAG, "Failed to read AHT20");
    }
    
    // Try to read BMP180
    esp_err_t bmp_ret = read_bmp180(&bmp_data);
    if (bmp_ret == ESP_OK) {
        data->pressure = bmp_data.pressure;
        // Use BMP180 temperature if AHT20 failed
        if (aht_ret != ESP_OK) {
            data->temperature = bmp_data.temperature;
        }
    } else {
        ESP_LOGE(TAG, "Failed to read BMP180");
    }
    
    ESP_LOGI(TAG, "Sensor Data - Temp: %.1f°C, Humidity: %.1f%%, Pressure: %.1f hPa", 
             data->temperature, data->humidity, data->pressure);
    
    // Simple relay auto control (removed for now to fix compilation)
    // TODO: Add auto relay control later
    
    return ESP_OK;
} 
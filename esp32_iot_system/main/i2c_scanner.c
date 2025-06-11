#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sensors.h"
#include "relay_control.h"

static const char *TAG = "I2C_SCANNER";

esp_err_t i2c_scanner_scan_devices(void)
{
    ESP_LOGI(TAG, "Starting I2C device scan...");
    ESP_LOGI(TAG, "     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    
    uint8_t devices_found = 0;
    
    for (uint8_t address = 1; address < 127; address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        
        if (address % 16 == 0) {
            printf("%02x: ", address);
        }
        
        if (ret == ESP_OK) {
            printf("%02x ", address);
            devices_found++;
            
            // Identify known devices
            switch (address) {
                case AHT20_ADDR:
                    ESP_LOGI(TAG, "Found AHT20 at address 0x%02X", address);
                    break;
                case BMP180_ADDR:
                    ESP_LOGI(TAG, "Found BMP180 at address 0x%02X", address);
                    break;
                case BMP180_ADDR_ALT:
                    ESP_LOGI(TAG, "Found BMP180 (ALT) at address 0x%02X", address);
                    break;
                default:
                    ESP_LOGI(TAG, "Found unknown device at address 0x%02X", address);
                    break;
            }
        } else {
            printf("-- ");
        }
        
        if ((address + 1) % 16 == 0) {
            printf("\n");
        }
    }
    
    if (devices_found == 0) {
        ESP_LOGE(TAG, "No I2C devices found!");
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "Found %d I2C device(s)", devices_found);
        return ESP_OK;
    }
}

esp_err_t i2c_scanner_test_sensors(void)
{
    ESP_LOGI(TAG, "Testing sensor readings...");
    
    // Test AHT20
    aht22_data_t aht20_data;
    esp_err_t ret = read_aht22(&aht20_data);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ AHT20 test PASSED");
    } else {
        ESP_LOGE(TAG, "❌ AHT20 test FAILED");
    }
    
    
    // Test BMP180
    bmp180_data_t bmp180_data;
    ret = read_bmp180(&bmp180_data);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ BMP180 test PASSED");
    } else {
        ESP_LOGE(TAG, "❌ BMP180 test FAILED");
    }
    
    // Combined sensor data test
    sensor_data_t sensor_data;
    ret = get_sensor_data(&sensor_data);
    if (ret == ESP_OK) {
            ESP_LOGI(TAG, "✅ Combined sensor test PASSED");
    ESP_LOGI(TAG, "   Temperature: %.1f°C", sensor_data.temperature);
    ESP_LOGI(TAG, "   Humidity: %.1f%%", sensor_data.humidity);
    ESP_LOGI(TAG, "   Pressure: %.1f hPa", sensor_data.pressure);
    
    // Test relay control
    ESP_LOGI(TAG, "========== RELAY TEST ==========");
    ESP_LOGI(TAG, "Testing RELAY_1 (GPIO47)...");
    relay_set_state(1, true);   // Turn ON relay 1
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2 seconds
    relay_set_state(1, false);  // Turn OFF relay 1
    
    ESP_LOGI(TAG, "Testing RELAY_2 (GPIO46)...");
    relay_set_state(2, true);   // Turn ON relay 2  
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2 seconds
    relay_set_state(2, false);  // Turn OFF relay 2
    ESP_LOGI(TAG, "✅ Relay test completed");
    } else {
        ESP_LOGE(TAG, "❌ Combined sensor test FAILED");
    }
    
    return ESP_OK;
}

void i2c_scanner_task(void *pvParameters)
{
    ESP_LOGI(TAG, "I2C Scanner Task Started");
    
    while (1) {
        // Scan for devices every 30 seconds
        ESP_LOGI(TAG, "========== I2C DEVICE SCAN ==========");
        i2c_scanner_scan_devices();
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
        
        // Test sensor readings every 30 seconds  
        ESP_LOGI(TAG, "========== SENSOR TEST ==========");
        i2c_scanner_test_sensors();
        
        ESP_LOGI(TAG, "========== WAITING 30s ==========");
        vTaskDelay(pdMS_TO_TICKS(25000)); // Wait 25 more seconds (total 30s)
    }
} 
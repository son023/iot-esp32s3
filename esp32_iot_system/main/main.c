#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_manager.h"
#include "web_server.h"
#include "sensors.h"
#include "relay_control.h"
#include "nvs_storage.h"

static const char *TAG = "MAIN";

// Task for automatic sensor reading and relay control
void sensor_auto_control_task(void *pvParameters)
{
    sensor_data_t data;
    
    while (1) {
        esp_err_t ret = get_sensor_data(&data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read sensor data");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 IoT System");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    storage_init();
    wifi_init();
    sensors_init();
    relay_init();
    
    uint8_t saved_auto_mode;
    storage_load_auto_mode(&saved_auto_mode);
    set_relay_mode(saved_auto_mode);
    
    uint8_t saved_relay_state;
    storage_load_relay_state(&saved_relay_state);
    set_relay_state(saved_relay_state);
    
    float temp_high, temp_low;
    storage_load_temp_thresholds(&temp_high, &temp_low);
    ESP_LOGI(TAG, "Loaded temperature thresholds: High=%.1f°C, Low=%.1f°C", temp_high, temp_low);
    init_webserver();

    xTaskCreate(sensor_auto_control_task, "sensor_auto", 4096, NULL, 4, NULL);
    ESP_LOGI(TAG, "Sensor auto control task started");

    ESP_LOGI(TAG, "System initialized successfully");
} 
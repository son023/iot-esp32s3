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
#include "i2c_scanner.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 IoT System");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize storage
    storage_init();

    // Initialize WiFi
    wifi_init();

    // Initialize sensors
    sensors_init();

    // Initialize relay control
    relay_init();
    
    // Load saved configurations
    uint8_t saved_auto_mode;
    storage_load_auto_mode(&saved_auto_mode);
    set_relay_mode(saved_auto_mode);
    
    uint8_t saved_relay_state;
    storage_load_relay_state(&saved_relay_state);
    set_relay_state(saved_relay_state);

    // Start web server
    init_webserver();

    // Start I2C scanner task for debugging
    xTaskCreate(i2c_scanner_task, "i2c_scanner", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "I2C Scanner task started");

    ESP_LOGI(TAG, "System initialized successfully");
} 
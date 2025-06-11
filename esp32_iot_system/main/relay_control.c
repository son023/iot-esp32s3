#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "relay_control.h"
#include "nvs_storage.h"

static const char *TAG = "RELAY";
static bool relay_state = false;

// Main relay (Relay 1) control variables
static relay_mode_t main_relay_mode = RELAY_MODE_MANUAL;

esp_err_t relay_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << RELAY_1_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed");
        return ret;
    }

    // Set relay to OFF initially
    gpio_set_level(RELAY_1_PIN, 0);
    relay_state = false;

    ESP_LOGI(TAG, "Relay control initialized successfully");
    return ESP_OK;
}

// Single relay functions (Main relay only)
esp_err_t set_relay_state(uint8_t state)
{
    // Relay logic: HIGH = ON, LOW = OFF (theo schematic)
    gpio_set_level(RELAY_1_PIN, state ? 1 : 0);
    relay_state = state ? true : false;

    ESP_LOGI(TAG, "🔌 RELAY_1 (GPIO%d) set to %s", RELAY_1_PIN, state ? "ON" : "OFF");
    return ESP_OK;
}

uint8_t get_relay_state(void)
{
    return relay_state ? 1 : 0;
}

// Auto mode functions
esp_err_t set_relay_mode(relay_mode_t mode)
{
    main_relay_mode = mode;
    ESP_LOGI(TAG, "Relay mode set to %s", mode == RELAY_MODE_AUTO ? "AUTO" : "MANUAL");
    return ESP_OK;
}

relay_mode_t get_relay_mode(void)
{
    return main_relay_mode;
}

esp_err_t auto_control_relay(float temperature, float temp_high, float temp_low)
{
    if (main_relay_mode != RELAY_MODE_AUTO) {
        return ESP_OK; // Không làm gì nếu không ở chế độ auto
    }
    
    bool current_state = get_relay_state();
    bool new_state = current_state;
    
    // Logic điều khiển: Bật relay khi nhiệt độ > ngưỡng cao, tắt khi < ngưỡng thấp
    if (temperature >= temp_high && !current_state) {
        new_state = true;
        ESP_LOGI(TAG, "AUTO: Temperature %.1f°C >= %.1f°C, turning relay ON", temperature, temp_high);
    } else if (temperature <= temp_low && current_state) {
        new_state = false;
        ESP_LOGI(TAG, "AUTO: Temperature %.1f°C <= %.1f°C, turning relay OFF", temperature, temp_low);
    }
    
    // Cập nhật relay nếu có thay đổi
    if (new_state != current_state) {
        set_relay_state(new_state ? 1 : 0);
        // Lưu state mới vào NVS khi auto control thay đổi
        storage_save_relay_state(new_state ? 1 : 0);
        ESP_LOGI(TAG, "AUTO: Relay state changed to %s and saved to NVS", new_state ? "ON" : "OFF");
    }
    
    return ESP_OK;
} 
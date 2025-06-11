#include "nvs_storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS_STORAGE";
static nvs_handle_t storage_handle;

esp_err_t storage_init(void)
{
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &storage_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "NVS storage initialized");
    return ESP_OK;
}

esp_err_t storage_save_relay_state(uint8_t state)
{
    esp_err_t err = nvs_set_u8(storage_handle, RELAY_STATE_KEY, state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving relay state: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_commit(storage_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing relay state: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Relay state saved: %d", state);
    return ESP_OK;
}

esp_err_t storage_load_relay_state(uint8_t* state)
{
    esp_err_t err = nvs_get_u8(storage_handle, RELAY_STATE_KEY, state);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Relay state not found, setting default to 0");
        *state = 0;
        return ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading relay state: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Relay state loaded: %d", *state);
    return ESP_OK;
}

esp_err_t storage_save_auto_mode(uint8_t auto_mode)
{
    esp_err_t err = nvs_set_u8(storage_handle, AUTO_MODE_KEY, auto_mode);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving auto mode: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_commit(storage_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing auto mode: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Auto mode saved: %d", auto_mode);
    return ESP_OK;
}

esp_err_t storage_load_auto_mode(uint8_t* auto_mode)
{
    esp_err_t err = nvs_get_u8(storage_handle, AUTO_MODE_KEY, auto_mode);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Auto mode not found, setting default to 0 (manual)");
        *auto_mode = 0;  // Mặc định là chế độ manual
        return ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading auto mode: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Auto mode loaded: %d", *auto_mode);
    return ESP_OK;
}

esp_err_t storage_save_temp_thresholds(float temp_high, float temp_low)
{
    esp_err_t err;
    
    err = nvs_set_blob(storage_handle, TEMP_THRESHOLD_HIGH_KEY, &temp_high, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving high temp threshold: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_set_blob(storage_handle, TEMP_THRESHOLD_LOW_KEY, &temp_low, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving low temp threshold: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_commit(storage_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing temp thresholds: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Temperature thresholds saved: High=%.1f°C, Low=%.1f°C", temp_high, temp_low);
    return ESP_OK;
}

esp_err_t storage_load_temp_thresholds(float* temp_high, float* temp_low)
{
    size_t required_size = sizeof(float);
    esp_err_t err;
    
    // Đọc ngưỡng cao
    err = nvs_get_blob(storage_handle, TEMP_THRESHOLD_HIGH_KEY, temp_high, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "High temp threshold not found, setting default to 30°C");
        *temp_high = 30.0;  // Mặc định 30°C
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading high temp threshold: %s", esp_err_to_name(err));
        return err;
    }
    
    // Đọc ngưỡng thấp
    required_size = sizeof(float);
    err = nvs_get_blob(storage_handle, TEMP_THRESHOLD_LOW_KEY, temp_low, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Low temp threshold not found, setting default to 25°C");
        *temp_low = 25.0;   // Mặc định 25°C
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading low temp threshold: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Temperature thresholds loaded: High=%.1f°C, Low=%.1f°C", *temp_high, *temp_low);
    return ESP_OK;
} 
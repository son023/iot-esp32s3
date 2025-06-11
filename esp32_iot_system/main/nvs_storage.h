#ifndef NVS_STORAGE_H
#define NVS_STORAGE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Storage keys
#define STORAGE_NAMESPACE "iot_config"
#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define RELAY_STATE_KEY "relay_state"
#define SENSOR_INTERVAL_KEY "sensor_interval"
#define AUTO_MODE_KEY "auto_mode"
#define TEMP_THRESHOLD_HIGH_KEY "temp_high"
#define TEMP_THRESHOLD_LOW_KEY "temp_low"

/**
 * @brief Initialize NVS storage
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_init(void);

/**
 * @brief Save WiFi credentials to NVS
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_save_wifi_credentials(const char* ssid, const char* password);

/**
 * @brief Load WiFi credentials from NVS
 * 
 * @param ssid Buffer to store SSID (must be at least 33 bytes)
 * @param password Buffer to store password (must be at least 65 bytes)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_load_wifi_credentials(char* ssid, char* password);

/**
 * @brief Save relay state to NVS
 * 
 * @param state Relay state (0 or 1)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_save_relay_state(uint8_t state);

/**
 * @brief Load relay state from NVS
 * 
 * @param state Pointer to store relay state
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_load_relay_state(uint8_t* state);

/**
 * @brief Save sensor reading interval to NVS
 * 
 * @param interval_ms Interval in milliseconds
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_save_sensor_interval(uint32_t interval_ms);

/**
 * @brief Load sensor reading interval from NVS
 * 
 * @param interval_ms Pointer to store interval
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_load_sensor_interval(uint32_t* interval_ms);

/**
 * @brief Save auto mode setting to NVS
 * 
 * @param auto_mode Auto mode enabled (1) or disabled (0)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_save_auto_mode(uint8_t auto_mode);

/**
 * @brief Load auto mode setting from NVS
 * 
 * @param auto_mode Pointer to store auto mode
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_load_auto_mode(uint8_t* auto_mode);

/**
 * @brief Save temperature thresholds to NVS
 * 
 * @param temp_high High temperature threshold (°C)
 * @param temp_low Low temperature threshold (°C)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_save_temp_thresholds(float temp_high, float temp_low);

/**
 * @brief Load temperature thresholds from NVS
 * 
 * @param temp_high Pointer to store high temperature threshold
 * @param temp_low Pointer to store low temperature threshold
 * @return esp_err_t ESP_OK on success
 */
esp_err_t storage_load_temp_thresholds(float* temp_high, float* temp_low);

#ifdef __cplusplus
}
#endif

#endif // NVS_STORAGE_H 
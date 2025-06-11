#ifndef NVS_STORAGE_H
#define NVS_STORAGE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_NAMESPACE "iot_config"
#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define RELAY_STATE_KEY "relay_state"
#define SENSOR_INTERVAL_KEY "sensor_interval"
#define AUTO_MODE_KEY "auto_mode"
#define TEMP_THRESHOLD_HIGH_KEY "temp_high"
#define TEMP_THRESHOLD_LOW_KEY "temp_low"


esp_err_t storage_init(void);

esp_err_t storage_save_relay_state(uint8_t state);
esp_err_t storage_load_relay_state(uint8_t* state);

esp_err_t storage_save_auto_mode(uint8_t auto_mode);
esp_err_t storage_load_auto_mode(uint8_t* auto_mode);

esp_err_t storage_save_temp_thresholds(float temp_high, float temp_low);
esp_err_t storage_load_temp_thresholds(float* temp_high, float* temp_low);

#ifdef __cplusplus
}
#endif

#endif
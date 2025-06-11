#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include "esp_err.h"

#define RELAY_1_PIN     47    // IO47 -> RELAY_1

typedef enum {
    RELAY_MODE_MANUAL = 0,
    RELAY_MODE_AUTO = 1
} relay_mode_t;

esp_err_t relay_init(void);

// Single relay functions (for main relay)
esp_err_t set_relay_state(uint8_t state);
uint8_t get_relay_state(void);

// Auto mode functions
esp_err_t set_relay_mode(relay_mode_t mode);
relay_mode_t get_relay_mode(void);
esp_err_t auto_control_relay(float temperature, float temp_high, float temp_low);

#endif 
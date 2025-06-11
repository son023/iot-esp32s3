#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include "esp_err.h"

/**
 * @brief Scan I2C bus for connected devices
 * @return ESP_OK if devices found, ESP_FAIL if no devices
 */
esp_err_t i2c_scanner_scan_devices(void);

/**
 * @brief Test sensor readings
 * @return ESP_OK on success
 */
esp_err_t i2c_scanner_test_sensors(void);

/**
 * @brief I2C scanner task - continuously scans and tests sensors
 * @param pvParameters Task parameters (unused)
 */
void i2c_scanner_task(void *pvParameters);

#endif // I2C_SCANNER_H 
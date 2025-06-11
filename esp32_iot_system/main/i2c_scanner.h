#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include "esp_err.h"

esp_err_t i2c_scanner_scan_devices(void);

void i2c_scanner_task(void *pvParameters);

#endif
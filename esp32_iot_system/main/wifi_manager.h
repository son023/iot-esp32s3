#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"

#define WIFI_SSID "TANG4"
#define WIFI_PASS "123456789@"

void wifi_init(void);
bool wifi_is_connected(void);

#endif 
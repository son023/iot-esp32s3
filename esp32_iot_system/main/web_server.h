#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_webserver(void);
void stop_webserver(httpd_handle_t server);

#ifdef __cplusplus
}
#endif

#endif 
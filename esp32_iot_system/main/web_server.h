#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start the web server
 */
void init_webserver(void);

/**
 * @brief Stop the web server
 * 
 * @param server HTTP server handle
 */
void stop_webserver(httpd_handle_t server);

#ifdef __cplusplus
}
#endif

#endif 
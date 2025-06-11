#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"

#include "web_server.h"
#include "sensors.h"
#include "relay_control.h"
#include "nvs_storage.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static const char *TAG = "WEB_SERVER";
static httpd_handle_t server = NULL;

// External declarations for embedded files
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[]   asm("_binary_style_css_end");
extern const uint8_t script_js_start[] asm("_binary_script_js_start");
extern const uint8_t script_js_end[]   asm("_binary_script_js_end");

// HTTP GET handler for root
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const size_t index_html_size = (index_html_end - index_html_start);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_size);
    return ESP_OK;
}

// HTTP GET handler for CSS
static esp_err_t style_get_handler(httpd_req_t *req)
{
    const size_t style_css_size = (style_css_end - style_css_start);
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)style_css_start, style_css_size);
    return ESP_OK;
}

// HTTP GET handler for JavaScript
static esp_err_t script_get_handler(httpd_req_t *req)
{
    const size_t script_js_size = (script_js_end - script_js_start);
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)script_js_start, script_js_size);
    return ESP_OK;
}

// HTTP GET handler for sensor data API
static esp_err_t api_sensors_get_handler(httpd_req_t *req)
{
    sensor_data_t data;
    esp_err_t ret = get_sensor_data(&data);
    
    if (ret != ESP_OK) {
        // Return default values if sensor read fails
        data.aht22_temperature = 25.0;
        data.aht22_humidity = 50.0;
        data.aht22_available = false;
        data.bmp180_temperature = 25.0;
        data.bmp180_pressure = 1013.25;
        data.bmp180_available = false;
        data.timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
    
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }
    
    // AHT22 sensor data
    cJSON *aht22 = cJSON_CreateObject();
    cJSON_AddNumberToObject(aht22, "temperature", data.aht22_temperature);
    cJSON_AddNumberToObject(aht22, "humidity", data.aht22_humidity);
    cJSON_AddBoolToObject(aht22, "available", data.aht22_available);
    cJSON_AddItemToObject(json, "aht22", aht22);
    
    // BMP180 sensor data
    cJSON *bmp180 = cJSON_CreateObject();
    cJSON_AddNumberToObject(bmp180, "temperature", data.bmp180_temperature);
    cJSON_AddNumberToObject(bmp180, "pressure", data.bmp180_pressure);
    cJSON_AddBoolToObject(bmp180, "available", data.bmp180_available);
    cJSON_AddItemToObject(json, "bmp180", bmp180);
    
    cJSON_AddNumberToObject(json, "timestamp", data.timestamp);
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    free(json_string);
    cJSON_Delete(json);
    return ESP_OK;
}

// HTTP GET handler for relay status API
static esp_err_t api_relay_get_handler(httpd_req_t *req)
{
    uint8_t relay_state = get_relay_state();
    relay_mode_t relay_mode = get_relay_mode();
    
    // Load thresholds for display
    float temp_high = 30.0, temp_low = 20.0;  // Default values
    storage_load_temp_thresholds(&temp_high, &temp_low);
    
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }
    
    cJSON_AddNumberToObject(json, "state", relay_state);
    cJSON_AddNumberToObject(json, "mode", relay_mode);
    cJSON_AddNumberToObject(json, "threshold_high", temp_high);
    cJSON_AddNumberToObject(json, "threshold_low", temp_low);
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    free(json_string);
    cJSON_Delete(json);
    return ESP_OK;
}

// HTTP POST handler for relay control API
static esp_err_t api_relay_post_handler(httpd_req_t *req)
{
    char content[200];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    
    content[ret] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *state_json = cJSON_GetObjectItem(json, "state");
    cJSON *mode_json = cJSON_GetObjectItem(json, "mode");
    
    // Handle state change
    if (cJSON_IsNumber(state_json)) {
        int new_state = (int)cJSON_GetNumberValue(state_json);
        if (new_state != 0 && new_state != 1) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "State must be 0 or 1");
            cJSON_Delete(json);
            return ESP_FAIL;
        }
        
        // Only allow state change in manual mode
        if (get_relay_mode() == RELAY_MODE_MANUAL) {
            set_relay_state((uint8_t)new_state);
            storage_save_relay_state((uint8_t)new_state);
        }
    }
    
    // Handle mode change
    if (cJSON_IsNumber(mode_json)) {
        int new_mode = (int)cJSON_GetNumberValue(mode_json);
        if (new_mode != 0 && new_mode != 1) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Mode must be 0 (manual) or 1 (auto)");
            cJSON_Delete(json);
            return ESP_FAIL;
        }
        
        set_relay_mode((relay_mode_t)new_mode);
        storage_save_auto_mode((uint8_t)new_mode);
    }
    
    cJSON *response = cJSON_CreateObject();
    if (response == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }
    
    cJSON_AddBoolToObject(response, "success", true);
    cJSON_AddNumberToObject(response, "state", get_relay_state());
    cJSON_AddNumberToObject(response, "mode", get_relay_mode());
    
    char *response_string = cJSON_Print(response);
    if (response_string == NULL) {
        cJSON_Delete(response);
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, response_string, HTTPD_RESP_USE_STRLEN);
    
    free(response_string);
    cJSON_Delete(response);
    cJSON_Delete(json);
    return ESP_OK;
}

// HTTP POST handler for temperature thresholds API
static esp_err_t api_thresholds_post_handler(httpd_req_t *req)
{
    char content[200];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    
    content[ret] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *temp_high_json = cJSON_GetObjectItem(json, "temp_high");
    cJSON *temp_low_json = cJSON_GetObjectItem(json, "temp_low");
    
    if (!cJSON_IsNumber(temp_high_json) || !cJSON_IsNumber(temp_low_json)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid temperature values");
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    
    float temp_high = (float)cJSON_GetNumberValue(temp_high_json);
    float temp_low = (float)cJSON_GetNumberValue(temp_low_json);
    
    // Validate thresholds
    if (temp_high <= temp_low) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "High temperature must be greater than low temperature");
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    
    if (temp_high < 0 || temp_high > 100 || temp_low < 0 || temp_low > 100) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Temperature must be between 0-100°C");
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    
    esp_err_t err = storage_save_temp_thresholds(temp_high, temp_low);
    
    cJSON *response = cJSON_CreateObject();
    if (response == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }
    
    cJSON_AddBoolToObject(response, "success", err == ESP_OK);
    cJSON_AddNumberToObject(response, "temp_high", temp_high);
    cJSON_AddNumberToObject(response, "temp_low", temp_low);
    
    if (err == ESP_OK) {
        cJSON_AddStringToObject(response, "message", "Temperature thresholds saved successfully");
    }
    
    char *response_string = cJSON_Print(response);
    if (response_string == NULL) {
        cJSON_Delete(response);
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, response_string, HTTPD_RESP_USE_STRLEN);
    
    free(response_string);
    cJSON_Delete(response);
    cJSON_Delete(json);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler
        };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t style = {
            .uri       = "/style.css",
            .method    = HTTP_GET,
            .handler   = style_get_handler
        };
        httpd_register_uri_handler(server, &style);

        httpd_uri_t script = {
            .uri       = "/script.js",
            .method    = HTTP_GET,
            .handler   = script_get_handler
        };
        httpd_register_uri_handler(server, &script);

        httpd_uri_t api_sensors = {
            .uri       = "/api/sensors",
            .method    = HTTP_GET,
            .handler   = api_sensors_get_handler
        };
        httpd_register_uri_handler(server, &api_sensors);

        httpd_uri_t api_relay_get = {
            .uri       = "/api/relay",
            .method    = HTTP_GET,
            .handler   = api_relay_get_handler
        };
        httpd_register_uri_handler(server, &api_relay_get);

        httpd_uri_t api_relay_post = {
            .uri       = "/api/relay",
            .method    = HTTP_POST,
            .handler   = api_relay_post_handler
        };
        httpd_register_uri_handler(server, &api_relay_post);

        httpd_uri_t api_thresholds = {
            .uri       = "/api/thresholds",
            .method    = HTTP_POST,
            .handler   = api_thresholds_post_handler
        };
        httpd_register_uri_handler(server, &api_thresholds);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void init_webserver(void)
{
    server = start_webserver();
    if (server) {
        ESP_LOGI(TAG, "Web server started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
} 
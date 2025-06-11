#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>

// Mock ESP-IDF types and functions
#define ESP_OK 0
#define ESP_FAIL -1
typedef int esp_err_t;
typedef uint32_t TickType_t;

// Mock sensor data structure
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    uint32_t timestamp;
} sensor_data_t;

// Mock relay mode
typedef enum {
    RELAY_MODE_MANUAL = 0,
    RELAY_MODE_AUTO = 1
} relay_mode_t;

// Global simulation variables
static relay_mode_t current_mode = RELAY_MODE_MANUAL;
static uint8_t relay_state = 0;
static float temp_high_threshold = 30.0;
static float temp_low_threshold = 25.0;

// Mock functions
void mock_log(const char* tag, const char* format, ...) {
    printf("[%s] ", tag);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Simulated sensor functions
esp_err_t mock_get_sensor_data(sensor_data_t *data) {
    // Simulate changing temperature
    static float temp = 26.0;
    static int direction = 1;
    
    temp += direction * 0.5;
    if (temp > 35.0) direction = -1;
    if (temp < 20.0) direction = 1;
    
    data->temperature = temp;
    data->humidity = 55.0 + (rand() % 20);
    data->pressure = 1013.25 + (rand() % 50 - 25);
    data->timestamp = time(NULL);
    
    return ESP_OK;
}

// Simulated relay functions
esp_err_t mock_set_relay_state(uint8_t state) {
    relay_state = state;
    printf("🔌 RELAY: %s\n", state ? "ON" : "OFF");
    return ESP_OK;
}

uint8_t mock_get_relay_state(void) {
    return relay_state;
}

esp_err_t mock_set_relay_mode(relay_mode_t mode) {
    current_mode = mode;
    printf("⚙️ MODE: %s\n", mode == RELAY_MODE_AUTO ? "AUTO" : "MANUAL");
    return ESP_OK;
}

relay_mode_t mock_get_relay_mode(void) {
    return current_mode;
}

// Auto control logic (copied from our implementation)
esp_err_t mock_auto_control_relay(float temperature, float temp_high, float temp_low) {
    if (current_mode != RELAY_MODE_AUTO) {
        return ESP_OK; // Không làm gì nếu không ở chế độ auto
    }
    
    bool current_state = mock_get_relay_state();
    bool new_state = current_state;
    
    // Logic điều khiển: Bật relay khi nhiệt độ > ngưỡng cao, tắt khi < ngưỡng thấp
    if (temperature >= temp_high && !current_state) {
        new_state = true;
        printf("🤖 AUTO: Temperature %.1f°C >= %.1f°C, turning relay ON\n", temperature, temp_high);
    } else if (temperature <= temp_low && current_state) {
        new_state = false;
        printf("🤖 AUTO: Temperature %.1f°C <= %.1f°C, turning relay OFF\n", temperature, temp_low);
    }
    
    // Cập nhật relay nếu có thay đổi
    if (new_state != current_state) {
        mock_set_relay_state(new_state ? 1 : 0);
        printf("🔄 AUTO: Relay state changed to %s\n", new_state ? "ON" : "OFF");
    }
    
    return ESP_OK;
}

// Test scenario
void test_auto_control_scenario() {
    printf("\n=== 🧪 ESP32 IoT System Auto Control Test ===\n\n");
    
    sensor_data_t sensor_data;
    
    // Test 1: Manual mode
    printf("📋 Test 1: Manual Mode\n");
    mock_set_relay_mode(RELAY_MODE_MANUAL);
    mock_set_relay_state(0);
    
    for (int i = 0; i < 3; i++) {
        mock_get_sensor_data(&sensor_data);
        printf("🌡️ Temperature: %.1f°C, Humidity: %.1f%%, Pressure: %.1f hPa\n", 
               sensor_data.temperature, sensor_data.humidity, sensor_data.pressure);
        mock_auto_control_relay(sensor_data.temperature, temp_high_threshold, temp_low_threshold);
        printf("📊 Relay State: %s\n", mock_get_relay_state() ? "ON" : "OFF");
        printf("---\n");
    }
    
    // Test 2: Auto mode
    printf("\n📋 Test 2: Auto Mode\n");
    mock_set_relay_mode(RELAY_MODE_AUTO);
    mock_set_relay_state(0);
    
    // Simulate temperature changes
    float test_temps[] = {28.0, 31.0, 32.0, 29.0, 24.0, 22.0, 26.0, 33.0};
    int num_tests = sizeof(test_temps) / sizeof(test_temps[0]);
    
    for (int i = 0; i < num_tests; i++) {
        sensor_data.temperature = test_temps[i];
        sensor_data.humidity = 55.0;
        sensor_data.pressure = 1013.25;
        sensor_data.timestamp = time(NULL);
        
        printf("🌡️ Temperature: %.1f°C (Thresholds: High=%.1f°C, Low=%.1f°C)\n", 
               sensor_data.temperature, temp_high_threshold, temp_low_threshold);
        
        mock_auto_control_relay(sensor_data.temperature, temp_high_threshold, temp_low_threshold);
        printf("📊 Relay State: %s\n", mock_get_relay_state() ? "ON" : "OFF");
        printf("---\n");
    }
    
    // Test 3: Threshold changes
    printf("\n📋 Test 3: Different Thresholds\n");
    temp_high_threshold = 35.0;
    temp_low_threshold = 30.0;
    mock_set_relay_state(0);
    
    printf("🔧 Changed thresholds: High=%.1f°C, Low=%.1f°C\n", temp_high_threshold, temp_low_threshold);
    
    float test_temps2[] = {32.0, 36.0, 29.0};
    int num_tests2 = sizeof(test_temps2) / sizeof(test_temps2[0]);
    
    for (int i = 0; i < num_tests2; i++) {
        sensor_data.temperature = test_temps2[i];
        printf("🌡️ Temperature: %.1f°C\n", sensor_data.temperature);
        mock_auto_control_relay(sensor_data.temperature, temp_high_threshold, temp_low_threshold);
        printf("📊 Relay State: %s\n", mock_get_relay_state() ? "ON" : "OFF");
        printf("---\n");
    }
    
    printf("\n✅ All tests completed!\n");
}

int main() {
    srand(time(NULL));
    test_auto_control_scenario();
    return 0;
} 
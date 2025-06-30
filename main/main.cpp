#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "i2c_manager.h"
#include "web_socket_client.h"
#include "mpu6500.h"
#include "WifiManager.h" 

static const char *TAG = "main";  // Tag for logging

// WiFi configuration 
#define WIFI_SSID      "BwagonRtale"
#define WIFI_PASSWORD  "1957-Rosa"

extern "C" void app_main()
{
    esp_err_t err;

    // Initialize Wifi Manager
    ESP_LOGI(TAG, "Initializing WiFi Manager");
    WiFiManager wifi;
    
    // Set up WiFi status callback 
    wifi.setStatusCallback([](WiFiManager::Status status, const std::string& message) {
        switch (status) {
            case WiFiManager::Status::CONNECTING:
                ESP_LOGI(TAG, "WiFi: Connecting...");
                break;
            case WiFiManager::Status::CONNECTED:
                ESP_LOGI(TAG, "WiFi: %s", message.c_str());
                break;
            case WiFiManager::Status::FAILED:
                ESP_LOGE(TAG, "WiFi: %s", message.c_str());
                break;
            case WiFiManager::Status::DISCONNECTED:
                ESP_LOGI(TAG, "WiFi: Disconnected");
                break;
        }
    });
    
    // Initialize WiFi subsystem
    if (wifi.init() != ESP_OK) {
        ESP_LOGE(TAG, "WiFi manager initialization failed");
        return;
    }
    
    // Connect to WiFi network
    if (wifi.connect(WIFI_SSID, WIFI_PASSWORD, 5) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
        return;
    }
    
    ESP_LOGI(TAG, "WiFi connected! IP: %s", wifi.getIPAddress().c_str());

    // Initialize the I2C manager
    err = i2c_manager_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C manager initialization failed");
        return;
    }
    
    // Create the MPU6500 object
    MPU6500 mpu;
    if (mpu.init() != ESP_OK) {
        ESP_LOGE(TAG, "MPU6500 initialization failed");
        return;
    }
    
    // Initialize and start the WebSocket client
    ws_client_start();
    
    // Send an initial message to indicate the ESP is ready
    ws_client_send("ESP32 MPU6500 WebSocket Data Stream Started at 100Hz");
    
    // Main loop: Read and log sensor data continuously
    char data_buffer[200];
    
    while (1) {
        // Check WiFi connection status periodically
        if (!wifi.isConnected()) {
            ESP_LOGW(TAG, "WiFi disconnected, attempting to reconnect...");
            wifi.connect(WIFI_SSID, WIFI_PASSWORD, 3);
        }
        
        float ax, ay, az, gx, gy, gz;

        if (mpu.read_data(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK) {
            char buffer[200];
            snprintf(buffer, sizeof(buffer),
                     "011 %.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
                     ax, ay, az, gx, gy, gz);
            ws_client_send(buffer);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/gpio.h"
#include "nvs_flash.h"

#include "WifiManager.h"
#include "web_socket_client.h"
#include "i2c_manager.h"
#include "MPU6500.h"

// Global variables
static const char *TAG = "main";

void mpu_reader_task(void *arg) {
    MPU6500 *mpu = static_cast<MPU6500*>(arg);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        float ax, ay, az, gx, gy, gz;
        esp_err_t result = mpu->read_data(&ax, &ay, &az, &gx, &gy, &gz);
        if (result == ESP_OK) {
            // Invert accel z axis 
            az = -az;
            // Create binary message
            uint8_t message[26]; // 1 sync + 1 flags + 24 data bytes (6 floats * 4 bytes)
            size_t msg_size = 0;
            
            // Sync byte
            message[msg_size++] = 0xAA;
            
            // Flags byte (0x03 = accel + gyro bits set)
            message[msg_size++] = 0x03; // 0x02 (accel) | 0x01 (gyro)
            
            // Data payload - accel first, then gyro
            // Convert floats to bytes (little endian)
            memcpy(&message[msg_size], &ax, 4); msg_size += 4;
            memcpy(&message[msg_size], &ay, 4); msg_size += 4;
            memcpy(&message[msg_size], &az, 4); msg_size += 4;
            memcpy(&message[msg_size], &gx, 4); msg_size += 4;
            memcpy(&message[msg_size], &gy, 4); msg_size += 4;
            memcpy(&message[msg_size], &gz, 4); msg_size += 4;
            
            // Send binary data over web socket
            ws_client_send_binary(message, msg_size);
        } else {
            ESP_LOGE(TAG, "MPU read error: %s", esp_err_to_name(result));
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10)); // 100Hz
    }
    
    // Cleanup (should never reach here)
    delete mpu;
    vTaskDelete(NULL);
}

// Magnetometer reading task
void mag_reader_task(void *arg) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        // generate fake magnetometer data
        float mx = 1.0f, my = 2.0f, mz = 3.0f;
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20)); // 50Hz
    }
    
    vTaskDelete(NULL);
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting application...");

    // Initialize nvs flash and WiFi
    nvs_flash_init();
    ESP_LOGI(TAG, "Initializing WiFi...");
    esp_err_t wifi_ret = wifi_init_sta();
    if (wifi_ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi initialization failed, continuing without WiFi");
    }

    // Initialize web socket client
    ESP_LOGI(TAG, "Initializing web socket client...");
    ws_client_start();

    // Initialize I2C
    ESP_ERROR_CHECK(i2c_manager_init());
    
    // Initialize MPU on heap
    i2c_master_bus_handle_t bus_handle = i2c_manager_get_bus_handle();
    MPU6500* mpu = new MPU6500(0x68);
    esp_err_t err = mpu->init(bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MPU initialization failed: %s", esp_err_to_name(err));
        delete mpu;
        return;
    }

    // Create MPU reader task
    BaseType_t task_result = xTaskCreate(
        mpu_reader_task,        "mpu_reader",
        5120,                   mpu,
        3,                      NULL   // High priority
    ); 
    if (task_result != pdPASS) {
        ESP_LOGE(TAG, "MPU reader task creation failed!");
        delete mpu;
        return;
    }

    // Create magnetometer reader task
    task_result = xTaskCreate(
        mag_reader_task,        "mag_reader",
        4096,                   NULL,
        2,                      NULL   // Medium priority
    );
    if (task_result != pdPASS) {
        ESP_LOGE(TAG, "Mag reader task creation failed!");
        delete mpu;
        return;
    }

    // Main loop
    while(1){
        vTaskDelay(1000);
    }
}
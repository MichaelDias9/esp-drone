#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "i2c_manager.h"
#include "Mpu6500.h"
#include "USBComm.h"  

static const char *TAG = "main";

extern "C" void app_main() {
    // Initialize USB communication
    usb_comm_init(); 

    // Initialize I2C manager
    esp_err_t err = i2c_manager_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C manager initialization failed");
        return;
    }
    
    // Initialize MPU6500
    MPU6500 mpu;
    if (mpu.init() != ESP_OK) {
        ESP_LOGE(TAG, "MPU6500 initialization failed");
        return;
    }

    // Main sensor loop
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // 200Hz loop
    ESP_LOGI(TAG, "Starting main sensor loop");
    while (1) {
        float ax, ay, az, gx, gy, gz;
        if (mpu.read_data(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK) {
            char buffer[200];
            int len = snprintf(buffer, sizeof(buffer), "011 %.5f,%.5f,%.5f,%.5f,%.5f,%.5f\n", 
                               ax, ay, -az, gx, gy, gz);
            usb_send_data(buffer, len);  
        } else {
            ESP_LOGW(TAG, "Failed to read MPU6500 data"); 
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
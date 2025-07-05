#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "i2c_manager.h"
#include "MPU6500.h"
#include "USBComm.h"  

static const char *TAG = "main";

extern "C" void app_main() {
    // Initialize USB communication
    usb_comm_init(); 

    // Initialize I2C manager
    ESP_ERROR_CHECK(i2c_manager_init());
    
    // Scan for I2C devices
    ESP_ERROR_CHECK(i2c_manager_scan());
    
    // Get the I2C bus handle
    i2c_master_bus_handle_t bus_handle = i2c_manager_get_bus_handle();
    
    // Create MPU6500 instance
    MPU6500 mpu(0x68);  // or just MPU6500 mpu; for default address
    
    // Initialize the MPU6500
    esp_err_t err = mpu.init(bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU6500: %s", esp_err_to_name(err));
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
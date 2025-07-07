#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "i2c_manager.h"
#include "MPU6500.h"

static const char *TAG = "main";

extern "C" void app_main() {    
    ESP_ERROR_CHECK(i2c_manager_init());
    ESP_ERROR_CHECK(i2c_manager_scan());
    
    i2c_master_bus_handle_t bus_handle = i2c_manager_get_bus_handle();
    
    MPU6500 mpu(0x68);
    esp_err_t err = mpu.init(bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU6500: %s", esp_err_to_name(err));
        return;
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // 200Hz loop
    
    ESP_LOGI(TAG, "Starting main sensor loop");
    while (1) {
        float ax, ay, az, gx, gy, gz;
        if (mpu.read_data(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK) {
            ESP_LOGI(TAG, "Received MPU6500 data: ax=%f ay=%f az=%f gx=%f gy=%f gz=%f", ax, ay, az, gx, gy, gz);
        } else {
            ESP_LOGW(TAG, "Failed to read MPU6500 data");
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "i2c_manager.h"
#include "MPU6500.h"
#include "USBComm.h"

static const char *TAG = "main";

// Binary protocol structure
typedef struct {
    uint8_t flags;     // Bit 0: mag, Bit 1: accel, Bit 2: gyro
    uint8_t count;     // Number of floats following
    float data[6];     // Up to 6 floats (2 sensors * 3 axes)
} __attribute__((packed)) sensor_packet_t;

extern "C" void app_main() {
    usb_comm_init();
    
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
            sensor_packet_t packet;
            packet.flags = 0x06;  // Binary: 110 (bit 2=gyro, bit 1=accel, bit 0=mag)
            packet.count = 6;     // 6 floats
            
            packet.data[0] = ax;   // accel X
            packet.data[1] = ay;   // accel Y
            packet.data[2] = -az;  // accel Z
            packet.data[3] = gx;   // gyro X
            packet.data[4] = gy;   // gyro Y
            packet.data[5] = gz;   // gyro Z
            
            // Send binary packet using USBComm interface
            size_t packet_size = sizeof(uint8_t) * 2 + sizeof(float) * packet.count;
            usb_send_data((const char*)&packet, packet_size);
        } else {
            ESP_LOGW(TAG, "Failed to read MPU6500 data");
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
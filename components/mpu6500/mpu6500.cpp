#include "mpu6500.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_manager.h"
#include "sensor_calibration.h"

static const char *TAG = "MPU6500";

// Constructor
MPU6500::MPU6500(uint8_t address) : dev_addr(address) {}

// Register write helper
esp_err_t MPU6500::write_register(uint8_t reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

// Register read helper
esp_err_t MPU6500::read_register(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);  // Repeated start
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

// WHO_AM_I check
esp_err_t MPU6500::read_whoami(uint8_t *who_am_i) {
    return read_register(0x75, who_am_i, 1);
}

// Initialization
esp_err_t MPU6500::init() {
    uint8_t whoami;
    ESP_ERROR_CHECK(read_whoami(&whoami));
    
    if (whoami != 0x70) {
        ESP_LOGE(TAG, "Invalid WHO_AM_I: 0x%02X", whoami);
        return ESP_FAIL;
    }

    // Wake up device
    ESP_ERROR_CHECK(write_register(0x6B, 0x00));
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Configuration
    ESP_ERROR_CHECK(write_register(0x1A, 0x00));  // CONFIG
    ESP_ERROR_CHECK(write_register(0x1B, 0x18));  // GYRO_CONFIG (+/- 2000dps)
    ESP_ERROR_CHECK(write_register(0x1C, 0x08));  // ACCEL_CONFIG (+/- 4g)
    
    return ESP_OK;
}

// Data reading
esp_err_t MPU6500::read_data(float* accel_x, float* accel_y, float* accel_z,
                             float* gyro_x, float* gyro_y, float* gyro_z) {
    uint8_t data[14];
    ESP_ERROR_CHECK(read_register(0x3B, data, 14));

    // Accelerometer data (16-bit)
    int16_t ax = (data[0] << 8) | data[1];
    int16_t ay = (data[2] << 8) | data[3];
    int16_t az = (data[4] << 8) | data[5];
    
    // Gyroscope data (16-bit)
    int16_t gx = (data[8] << 8) | data[9];
    int16_t gy = (data[10] << 8) | data[11];
    int16_t gz = (data[12] << 8) | data[13];
    
    // Convert to physical values
    const float accel_scale = 4.0f / 32768.0f;  // ±4g range
    const float gyro_scale = 2000.0f / 32768.0f * (3.1415926535f / 180.0f);  // ±2000dps to rad/s
    
    *accel_x = ax * accel_scale;
    *accel_y = ay * accel_scale;
    *accel_z = az * accel_scale;
    
    *gyro_x = gx * gyro_scale;
    *gyro_y = gy * gyro_scale;
    *gyro_z = gz * gyro_scale;
    
    return ESP_OK;
}
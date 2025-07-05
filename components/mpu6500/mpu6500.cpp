#include "mpu6500.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_manager.h"

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
    /* Check whoami value (optional)
    uint8_t whoami;
    ESP_ERROR_CHECK(read_whoami(&whoami));
    ESP_LOGI(TAG, "WHO_AM_I: 0x%02X", whoami);
    if (whoami != 0x70) {
        ESP_LOGE(TAG, "Invalid WHO_AM_I: 0x%02X", whoami);
        return ESP_FAIL;
    }*/

    // 1. Wake up device (disable sleep, use best clock)
    ESP_ERROR_CHECK(write_register(PWR_MGMT_1, 0x01));      // Clock: PLL with X-axis gyro reference
    
    // 2. Configure Digital Low-Pass Filter (DLPF)
    ESP_ERROR_CHECK(write_register(CONFIG, 0x00));          // DLPF_CFG = 0 (Gyro: 250Hz BW)

    // 3. Configure sensors:
    ESP_ERROR_CHECK(write_register(GYRO_CONFIG, 0x00));     // Gyro: ±250°/s range
    ESP_ERROR_CHECK(write_register(ACCEL_CONFIG, 0x00));    // Accel: ±2g range
    
    // 4. Enable Accel DLPF (disable A_DLPF_CFG bypass)
    ESP_ERROR_CHECK(write_register(ACCEL_CONFIG2, 0x01));   // Accel Fchoice_b = 0 (enable DLPF) 184Hz

    // 5. Sample Rate = 1kHz / (1 + SMPLRT_DIV)
    ESP_ERROR_CHECK(write_register(SMPLRT_DIV, 0x04));      // Sample rate = 200Hz (1kHz/(1+4))
    
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
    const float accel_scale = 2.0f / 32768.0f;  // ±2g range
    const float gyro_scale = 250.0f / 32768.0f * (3.1415926535f / 180.0f);  // ±250dps to rad/s
    
    *accel_x = ax * accel_scale;
    *accel_y = ay * accel_scale;
    *accel_z = az * accel_scale;
    
    *gyro_x = gx * gyro_scale;
    *gyro_y = gy * gyro_scale;
    *gyro_z = gz * gyro_scale;
    
    return ESP_OK;
}
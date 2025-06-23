#include "mpu6500.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_manager.h"          // Use the I2C manager for bus access
#include "sensor_calibration.h"   // Use the sensor calibration factors 

static const char *TAG = "mpu6500";

// Helper function to write a value to a register
static esp_err_t mpu6500_write_register(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6500_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    return err;
}

// Helper function to read one or more bytes from a register
static esp_err_t mpu6500_read_register(uint8_t reg, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // Write the register address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6500_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    // Repeated start for reading
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6500_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    return err;
}

// Helper function to wake up the sensor
esp_err_t mpu6500_init(void)
{
    // Wake up the sensor by writing 0 to the power management register
    esp_err_t err = mpu6500_write_register(MPU6500_PWR_MGMT_1, 0x00);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6500");
        return err;
    }
    // Allow time for sensor stabilization
    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

// Helper function to read acceleration and gyro data
esp_err_t mpu6500_read_data(mpu6500_data_t *data)
{
    uint8_t accel_raw[6] = {0};
    uint8_t gyro_raw[6] = {0};
    
    esp_err_t err = mpu6500_read_register(MPU6500_ACCEL_XOUT_H, accel_raw, 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read accelerometer data");
        return err;
    }
    err = mpu6500_read_register(MPU6500_GYRO_XOUT_H, gyro_raw, 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read gyroscope data");
        return err;
    }
    
    // Combine high and low bytes for accelerometer
    int16_t raw_accel_x = (int16_t)((accel_raw[0] << 8) | accel_raw[1]);
    int16_t raw_accel_y = (int16_t)((accel_raw[2] << 8) | accel_raw[3]);
    int16_t raw_accel_z = (int16_t)((accel_raw[4] << 8) | accel_raw[5]);
    
    // Convert raw values to g's (assuming 16384 LSB/g) and include calibration factors
    float accel_x_g = (raw_accel_x / 16384.0f) * calibration.accel.scale_x + calibration.accel.offset_x;
    float accel_y_g = (raw_accel_y / 16384.0f) * calibration.accel.scale_y + calibration.accel.offset_y;
    float accel_z_g = (raw_accel_z / 16384.0f) * calibration.accel.scale_z + calibration.accel.offset_z;
    
    // Combine high and low bytes for gyroscope
    int16_t raw_gyro_x = (int16_t)((gyro_raw[0] << 8) | gyro_raw[1]);
    int16_t raw_gyro_y = (int16_t)((gyro_raw[2] << 8) | gyro_raw[3]);
    int16_t raw_gyro_z = (int16_t)((gyro_raw[4] << 8) | gyro_raw[5]);
    
    // Convert raw values to degrees per second (assuming 131 LSB/dps),
    // then apply scaling and offset from the calibration data
    float gyro_x_dps = (raw_gyro_x / 131.0f) * calibration.gyro.scale_x + calibration.gyro.offset_x;
    float gyro_y_dps = (raw_gyro_y / 131.0f) * calibration.gyro.scale_y + calibration.gyro.offset_y;
    float gyro_z_dps = (raw_gyro_z / 131.0f) * calibration.gyro.scale_z + calibration.gyro.offset_z;
    
    // Fill the sensor data structure
    data->accel_x = accel_x_g;
    data->accel_y = accel_y_g;
    data->accel_z = accel_z_g;
    data->gyro_x = gyro_x_dps;
    data->gyro_y = gyro_y_dps;
    data->gyro_z = gyro_z_dps;
    
    return ESP_OK;
}
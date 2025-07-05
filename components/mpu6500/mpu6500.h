#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include <stdint.h>

#define PWR_MGMT_1            0x6B
#define ACCEL_CONFIG          0x1C
#define GYRO_CONFIG           0x1B
#define SMPLRT_DIV            0x19
#define CONFIG                0x1A
#define INT_PIN_CFG           0x37
#define ACCEL_CONFIG2         0x1D

//Struct to hold accelerometer and gyroscope data.
 struct mpu6500_data_t {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};


class MPU6500 {
public:
    // Constructor with configurable I2C address
    MPU6500(uint8_t address = 0x68);

    // Initialization method
    esp_err_t init();

    // Read sensor data
    esp_err_t read_data(float* accel_x, float* accel_y, float* accel_z,
                        float* gyro_x, float* gyro_y, float* gyro_z);

private:
    uint8_t dev_addr;
    
    // Helper methods
    esp_err_t write_register(uint8_t reg, uint8_t value);
    esp_err_t read_register(uint8_t reg, uint8_t *data, size_t len);
    esp_err_t read_whoami(uint8_t *who_am_i);
};
#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// MPU6500 I2C address
#define MPU6500_I2C_ADDR 0x68

// MPU6500 register addresses
#define PWR_MGMT_1      0x6B
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_CONFIG2   0x1D
#define SMPLRT_DIV      0x19
#define WHO_AM_I        0x75

class MPU6500 {
private:
    uint8_t dev_addr;
    i2c_master_dev_handle_t dev_handle;
    
    esp_err_t write_register(uint8_t reg, uint8_t value);
    esp_err_t read_register(uint8_t reg, uint8_t *data, size_t len);

public:
    MPU6500(uint8_t address = MPU6500_I2C_ADDR);
    ~MPU6500();
    
    esp_err_t init(i2c_master_bus_handle_t bus_handle);
    esp_err_t deinit();
    esp_err_t read_whoami(uint8_t *who_am_i);
    esp_err_t read_data(float* accel_x, float* accel_y, float* accel_z,
                        float* gyro_x, float* gyro_y, float* gyro_z);
};

#ifdef __cplusplus
}
#endif
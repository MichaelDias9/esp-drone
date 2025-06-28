#ifndef MPU6500_H
#define MPU6500_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.14159265358979323846
#define LSB_TO_DEG_PER_SEC 131.0f
static const float LSB_TO_RAD_PER_SEC = PI / (180 * LSB_TO_DEG_PER_SEC);

// MPU6500 register addresses
#define MPU6500_ADDR               0x68
#define MPU6500_WHO_AM_I_REG       0x75
#define MPU6500_PWR_MGMT_1         0x6B
#define MPU6500_PWR_MGMT_2         0x6C
#define MPU6500_CONFIG             0x1A
#define MPU6500_GYRO_CONFIG        0x1B
#define MPU6500_ACCEL_CONFIG_1     0x1C
#define MPU6500_ACCEL_CONFIG_2     0x1D
#define MPU6500_ACCEL_XOUT_H       0x3B
#define MPU6500_GYRO_XOUT_H        0x43

/**
 * @brief Struct to hold accelerometer and gyroscope data.
 */
 struct mpu6500_data_t {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

/**
 * @brief Initialize the MPU6500 sensor (wakes up the device).
 *
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t init(void);

/**
 * @brief Read sensor data (accelerometer and gyroscope) from the MPU6500.
 *
 * @param data Pointer to a mpu6500_data_t structure to store the readings.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t read_data(mpu6500_data_t *data);

/**
 * @brief Read the WHO_AM_I register from the MPU6500.
 *
 * @param who_am_i Pointer to store the WHO_AM_I value.
 * @return ESP_OK on success, error  code otherwise.
 */
void test_whoami(uint8_t *who_am_i);

esp_err_t read_whoami(uint8_t *who_am_i);

#ifdef __cplusplus
}
#endif

#endif // MPU6500_H

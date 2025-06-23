#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C configuration macros
#define I2C_MASTER_NUM             I2C_NUM_0
#define I2C_MASTER_SDA_IO          21
#define I2C_MASTER_SCL_IO          22
#define I2C_MASTER_FREQ_HZ         100000
#define I2C_MASTER_TX_BUF_DISABLE  0
#define I2C_MASTER_RX_BUF_DISABLE  0
#define I2C_TIMEOUT_MS             1000

/**
 * @brief Initialize the I2C bus.
 *
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t i2c_manager_init(void);

/**
 * @brief Scan the I2C bus and log any found devices.
 *
 * @return ESP_OK on success.
 */
esp_err_t i2c_manager_scan(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_MANAGER_H

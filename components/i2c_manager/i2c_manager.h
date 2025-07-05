#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C configuration macros
#define I2C_MASTER_SDA_IO          21
#define I2C_MASTER_SCL_IO          22
#define I2C_MASTER_FREQ_HZ         100000
#define I2C_TIMEOUT_MS             1000

/**
 * @brief Initialize the I2C bus.
 *
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t i2c_manager_init(void);

/**
 * @brief Deinitialize the I2C bus.
 *
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t i2c_manager_deinit(void);

/**
 * @brief Scan the I2C bus and log any found devices.
 *
 * @return ESP_OK on success.
 */
esp_err_t i2c_manager_scan(void);

/**
 * @brief Get the I2C bus handle for use with devices.
 *
 * @return I2C bus handle, or NULL if not initialized.
 */
i2c_master_bus_handle_t i2c_manager_get_bus_handle(void);

#ifdef __cplusplus
}
#endif
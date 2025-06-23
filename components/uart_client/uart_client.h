#ifndef UART_CLIENT
#define UART_CLIENT

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the UART for serial communication
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t uart_client_init(void);

/**
 * @brief Send data over the UART interface
 * 
 * @param data Pointer to the data to send
 * @param len Length of the data in bytes
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t uart_client_send(const char *data, size_t len);

/**
 * @brief Deinitialize the UART interface
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t uart_client_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
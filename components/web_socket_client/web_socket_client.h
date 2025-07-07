#ifndef WEB_SOCKET_CLIENT_H
#define WEB_SOCKET_CLIENT_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Start the WebSocket client and connect to the server
 
void ws_client_start(void);

/**
 * @brief Send text data over the WebSocket connection
 *
 * @param data Null-terminated string to send
 */
void ws_client_send(const char *data);

/**
 * @brief Send binary data over the WebSocket connection
 *
 * @param data Pointer to binary data
 * @param len Length of binary data in bytes
 */
void ws_client_send_binary(const uint8_t *data, size_t len);

/**
 * @brief Stop the WebSocket client and clean up resources
 */
void ws_client_stop(void);

#ifdef __cplusplus
}
#endif

#endif // WEB_SOCKET_CLIENT_H
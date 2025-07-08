#ifndef WEB_SOCKET_CLIENT_H
#define WEB_SOCKET_CLIENT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the WebSocket client
 */
void ws_client_start(void);

/**
 * @brief Send text data over WebSocket
 * @param data Null-terminated string to send
 */
void ws_client_send(const char *data);

/**
 * @brief Send binary data over WebSocket
 * @param data Pointer to binary data buffer
 * @param length Length of binary data in bytes
 */
void ws_client_send_binary(const uint8_t *data, size_t length);

/**
 * @brief Stop the WebSocket client
 */
void ws_client_stop(void);

#ifdef __cplusplus
}
#endif

#endif // WEB_SOCKET_CLIENT_H
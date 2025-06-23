#include "uart_client.h"
#include "driver/uart.h"
#include <stdio.h>
#include <string.h>

#define UART_NUM            UART_NUM_0  // Use UART0 (usually connected to USB)
#define UART_BAUD_RATE      921600      // Increased baud rate for higher frequency data
#define UART_BUF_SIZE       2048        // Increased buffer size for faster data rate

esp_err_t uart_client_init(void)
{
    // Configure UART parameters
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    // Install UART driver
    printf("Initializing UART...\n");
    esp_err_t err = uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        printf("Failed to install UART driver: %d\n", err);
        return err;
    }
    
    // Configure UART parameters
    err = uart_param_config(UART_NUM, &uart_config);
    if (err != ESP_OK) {
        printf("Failed to configure UART parameters: %d\n", err);
        return err;
    }
    
    printf("UART initialized successfully at %d baud\n", UART_BAUD_RATE);
    return ESP_OK;
}

esp_err_t uart_client_send(const char *data, size_t len)
{
    if (data == NULL) {
        printf("NULL data pointer provided\n");
        return ESP_ERR_INVALID_ARG;
    }
    
    int txBytes = uart_write_bytes(UART_NUM, data, len);
    if (txBytes < 0) {
        printf("UART write error\n");
        return ESP_FAIL;
    } else if (txBytes != len) {
        printf("UART write incomplete: %d/%d bytes\n", txBytes, len);
        return ESP_ERR_INVALID_STATE;
    }
    
    // Add a line terminator for better readability on the receiving end
    uart_write_bytes(UART_NUM, "\r\n", 2);
    
    return ESP_OK;
}

esp_err_t uart_client_deinit(void)
{
    esp_err_t err = uart_driver_delete(UART_NUM);
    if (err != ESP_OK) {
        printf("Failed to delete UART driver: %d\n", err);
        return err;
    }
    
    printf("UART deinitialized\n");
    return ESP_OK;
}
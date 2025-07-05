#include "USBComm.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define UART_NUM UART_NUM_0
#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)
#define TX_BUFFER_SIZE 512

static const char* TAG = "USB_COMM";

void usb_comm_init() {
    // UART configuration
    uart_config_t uart_cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,  // Required in ESP-IDF v5.x
        .source_clk = UART_SCLK_APB,
        .flags = 0                 // Initialize flags field
    };

    // Install UART driver
    ESP_ERROR_CHECK(uart_driver_install(
        UART_NUM, 
        TX_BUFFER_SIZE,  // TX buffer size
        0,               // RX buffer size (not used)
        0,               // Queue size
        NULL,            // No event queue
        0                // No flags
    ));
    
    // Configure parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_cfg));
    
    // Set pins
    ESP_ERROR_CHECK(uart_set_pin(
        UART_NUM, 
        TXD_PIN, 
        RXD_PIN, 
        UART_PIN_NO_CHANGE,  // RTS not used
        UART_PIN_NO_CHANGE   // CTS not used
    ));
    
    ESP_LOGI(TAG, "USB UART initialized");
}

void usb_send_data(const char* data, size_t length) {
    // Write data to UART (non-blocking)
    uart_write_bytes(UART_NUM, data, length);
}
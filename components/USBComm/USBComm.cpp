#include "USBComm.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

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
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_APB,
        .flags = 0
    };

    ESP_ERROR_CHECK(uart_driver_install(
        UART_NUM, 
        TX_BUFFER_SIZE,
        0,
        0,
        NULL,
        0
    ));
    
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(
        UART_NUM, 
        TXD_PIN, 
        RXD_PIN, 
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    ));
    
    ESP_LOGI(TAG, "USB UART initialized");
}

void usb_send_data(const char* data, size_t length) {
    uart_write_bytes(UART_NUM, data, length);
}

void usb_send_binary_floats(const float* data, size_t count) {
    // Send binary data directly - no conversion needed
    uart_write_bytes(UART_NUM, (const char*)data, count * sizeof(float));
}
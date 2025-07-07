#pragma once

#include "esp_err.h"

// WiFi Configuration 
#define WIFI_SSID "AB8C2B-2.4"
#define WIFI_PASSWORD "CP222026624P"
#define WIFI_MAX_RETRIES 5

// WiFi event bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Initializes WiFi in STA mode and connects to a network.
// Returns ESP_OK on success, ESP_FAIL otherwise.
esp_err_t wifi_init_sta(void);

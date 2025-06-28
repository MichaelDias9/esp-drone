#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "i2c_manager.h"
#include "web_socket_client.h"
#include "mpu6500.h"

static const char *TAG = "main";

/* WiFi configuration - REPLACE WITH YOUR NETWORK CREDENTIALS */
#define WIFI_SSID      "BwagonRtale"
#define WIFI_PASSWORD  "1957-Rosa"
#define WIFI_MAXIMUM_RETRY  5

/* FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"Connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP SSID:%s", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "Unexpected event");
    }
}

void app_main(void)
{
    esp_err_t err;

    // Initialize NVS flash (required for WiFi)
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    
    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi");
    wifi_init_sta();

    // Initialize the I2C manager
    err = i2c_manager_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C manager initialization failed");
        return;
    }
    
    // Optionally, scan the I2C bus to list connected devices
    i2c_manager_scan();
    
    // Initialize the MPU6500 sensor
    err = mpu6500_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MPU6500 initialization failed");
        return;
    }
    
    // Initialize and start the WebSocket client
    ws_client_start();
    
    // Send an initial message to indicate the ESP is ready
    ws_client_send("ESP32 MPU6500 WebSocket Data Stream Started at 100Hz");
    
    // Main loop: Read and log sensor data continuously
    mpu6500_data_t sensor_data;
    char data_buffer[200];
    
    while (1) {
        err = mpu6500_read_data(&sensor_data);
        if (err == ESP_OK) {
            // Format the sensor data into the string - use minimal formatting to reduce overhead
            int len = snprintf(data_buffer, sizeof(data_buffer), 
                "011 %.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
                sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z,
                sensor_data.gyro_x, sensor_data.gyro_y, sensor_data.gyro_z);
          
            // Send the data over WebSocket
            ws_client_send(data_buffer);  
            
        } else {
            ESP_LOGE(TAG, "Failed to read sensor data");
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms delay for 100Hz update rate
    }
}
#include "web_socket_client.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ws_client";
static esp_websocket_client_handle_t client = NULL;

static void websocket_event_handler(void *args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket Connected");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WebSocket Disconnected");
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "WebSocket Error");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGD(TAG, "Received data: %.*s", data->data_len, (char *)data->data_ptr);
            break;
    }
}

void ws_client_start(void)
{
    esp_websocket_client_config_t ws_cfg = {
        .uri = "ws://192.168.4.58:8000",
        .disable_auto_reconnect = false,
        .reconnect_timeout_ms = 5000,
    };

    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
    esp_websocket_client_start(client);
}

void ws_client_send(const char *data)
{
    if (client && esp_websocket_client_is_connected(client)) {
        int ret = esp_websocket_client_send_text(client, data, strlen(data), pdMS_TO_TICKS(10));
        if (ret < 0) {
            ESP_LOGW(TAG, "WebSocket send failed: %d", ret);
        }
    }
}

void ws_client_stop(void)
{
    if (client) {
        esp_websocket_client_stop(client);
        esp_websocket_client_destroy(client);
        client = NULL;
    }
}
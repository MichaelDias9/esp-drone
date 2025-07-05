#include "WifiManager.h"

#include "sdkconfig.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <cstring>

static const char* TAG = "WiFiManager";

WiFiManager::WiFiManager() 
    : event_group_(nullptr)
    , status_(Status::DISCONNECTED)
    , retry_count_(0)
    , max_retries_(5)
    , status_callback_(nullptr)
{
}

WiFiManager::~WiFiManager() {
    disconnect();
    if (event_group_) {
        vEventGroupDelete(event_group_);
    }
}

esp_err_t WiFiManager::init() {
    // Initialize NVS flash (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS flash: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "NVS flash initialized");

    // Create event group  
    event_group_ = xEventGroupCreate();
    if (!event_group_) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                              &WiFiManager::event_handler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                              &WiFiManager::event_handler, this));

    ESP_LOGI(TAG, "WiFi manager initialized");
    return ESP_OK;
}

esp_err_t WiFiManager::connect(const std::string& ssid, const std::string& password, 
                              int max_retries) {
    if (ssid.empty()) {
        ESP_LOGE(TAG, "SSID cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }

    max_retries_ = max_retries;
    retry_count_ = 0;
    status_ = Status::CONNECTING;

    // Configure WiFi
    wifi_config_t wifi_config = {};
    
    // Safe copy of SSID and password
    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), 
            ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password), 
            password.c_str(), sizeof(wifi_config.sta.password) - 1);
    
    // Security settings
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", ssid.c_str());

    // Wait for connection result
    EventBits_t bits = xEventGroupWaitBits(event_group_,
                                          CONNECTED_BIT | FAIL_BIT,
                                          pdTRUE, // Clear bits after wait
                                          pdFALSE, // Wait for any bit
                                          portMAX_DELAY);

    if (bits & CONNECTED_BIT) {
        status_ = Status::CONNECTED;
        ESP_LOGI(TAG, "Connected to WiFi successfully");
        if (status_callback_) {
            status_callback_(status_, "Connected to " + ssid);
        }
        return ESP_OK;
    } else if (bits & FAIL_BIT) {
        status_ = Status::FAILED;
        ESP_LOGE(TAG, "Failed to connect to WiFi");
        if (status_callback_) {
            status_callback_(status_, "Failed to connect to " + ssid);
        }
        return ESP_FAIL;
    }

    return ESP_FAIL;
}

esp_err_t WiFiManager::disconnect() {
    if (status_ == Status::CONNECTED || status_ == Status::CONNECTING) {
        esp_wifi_disconnect();
        status_ = Status::DISCONNECTED;
        ip_address_.clear();
        ESP_LOGI(TAG, "Disconnected from WiFi");
    }
    return ESP_OK;
}

void WiFiManager::setCredentials(const std::string& ssid, const std::string& password) {
    stored_ssid_ = ssid;
    stored_password_ = password;
}

esp_err_t WiFiManager::reconnect(int max_retries) {
    if (stored_ssid_.empty() || stored_password_.empty()) {
        ESP_LOGE(TAG, "No stored credentials for reconnect");
        return ESP_ERR_INVALID_STATE;
    }
    return connect(stored_ssid_, stored_password_, max_retries);
}

void WiFiManager::event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    WiFiManager* manager = static_cast<WiFiManager*>(arg);
    manager->handleWiFiEvent(event_base, event_id, event_data);
}

void WiFiManager::handleWiFiEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_count_ < max_retries_) {
            esp_wifi_connect();
            retry_count_++;
            ESP_LOGI(TAG, "Retry connecting (%d/%d)", retry_count_, max_retries_);
        } else {
            xEventGroupSetBits(event_group_, FAIL_BIT);
            ESP_LOGE(TAG, "Maximum retry attempts reached");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(event_data);
        
        // Format IP address
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        ip_address_ = ip_str;
        
        retry_count_ = 0;
        xEventGroupSetBits(event_group_, CONNECTED_BIT);
        ESP_LOGI(TAG, "Got IP address: %s", ip_address_.c_str());
    }
}
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <string>
#include <functional>

class WiFiManager {
public:
    enum class Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        FAILED
    };

    using StatusCallback = std::function<void(Status, const std::string&)>;

    WiFiManager();
    ~WiFiManager();

    // Initialize WiFi subsystem
    esp_err_t init();
    
    // Connect to WiFi network
    esp_err_t connect(const std::string& ssid, const std::string& password, 
                     int max_retries = 5);
    
    // Disconnect from WiFi
    esp_err_t disconnect();

    // Connect using stored credentials
    esp_err_t reconnect(int max_retries = 5);
    
    // Store credentials
    void setCredentials(const std::string& ssid, const std::string& password);
    
    // Get current status
    Status getStatus() const { return status_; }
    
    // Get IP address as string
    std::string getIPAddress() const { return ip_address_; }
    
    // Set status callback
    void setStatusCallback(StatusCallback callback) { status_callback_ = callback; }
    
    // Check if connected
    bool isConnected() const { return status_ == Status::CONNECTED; }

private:
    static void event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data);
    
    void handleWiFiEvent(esp_event_base_t event_base, int32_t event_id, void* event_data);
    
    std::string stored_ssid_;
    std::string stored_password_;

    EventGroupHandle_t event_group_;
    Status status_;
    std::string ip_address_;
    int retry_count_;
    int max_retries_;
    StatusCallback status_callback_;
    
    static constexpr int CONNECTED_BIT = BIT0;
    static constexpr int FAIL_BIT = BIT1;
};  
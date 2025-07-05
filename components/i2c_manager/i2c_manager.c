#include "i2c_manager.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "i2c_manager";
static i2c_master_bus_handle_t i2c_bus_handle;

esp_err_t i2c_manager_init(void)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0, // 0 means use default
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err = i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C master bus creation failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C master bus initialized successfully");
    return ESP_OK;
}

esp_err_t i2c_manager_deinit(void)
{
    if (i2c_bus_handle != NULL) {
        esp_err_t err = i2c_del_master_bus(i2c_bus_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2C master bus deletion failed: %s", esp_err_to_name(err));
            return err;
        }
        i2c_bus_handle = NULL;
        ESP_LOGI(TAG, "I2C master bus deinitialized");
    }
    return ESP_OK;
}

esp_err_t i2c_manager_scan(void)
{
    if (i2c_bus_handle == NULL) {
        ESP_LOGE(TAG, "I2C not initialized. Call i2c_manager_init() first.");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Scanning I2C bus...");
    
    for (int address = 1; address < 127; address++) {
        // Create a temporary device handle for probing
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = address,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        };
        
        i2c_master_dev_handle_t dev_handle;
        esp_err_t err = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &dev_handle);
        if (err != ESP_OK) {
            continue;
        }
        
        // Try to probe the device by sending a zero-length write
        uint8_t dummy_data = 0;
        err = i2c_master_transmit(dev_handle, &dummy_data, 0, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Found device at address: 0x%02X", address);
        }
        
        // Clean up the device handle
        i2c_master_bus_rm_device(dev_handle);
    }
    
    ESP_LOGI(TAG, "I2C scan completed");
    return ESP_OK;
}

i2c_master_bus_handle_t i2c_manager_get_bus_handle(void)
{
    return i2c_bus_handle;
}
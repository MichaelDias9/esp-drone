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
        // Use the bus handle directly with i2c_master_probe
        esp_err_t err = i2c_master_probe(i2c_bus_handle, address, I2C_TIMEOUT_MS);
        
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Found device at address: 0x%02X", address);
        }
    }
    
    ESP_LOGI(TAG, "I2C scan completed");
    return ESP_OK;
}

i2c_master_bus_handle_t i2c_manager_get_bus_handle(void)
{
    return i2c_bus_handle;
}
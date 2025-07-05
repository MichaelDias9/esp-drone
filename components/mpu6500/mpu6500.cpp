#include "MPU6500.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_manager.h"

static const char *TAG = "MPU6500";

// Constructor
MPU6500::MPU6500(uint8_t address) : dev_addr(address), dev_handle(nullptr) {}

// Destructor
MPU6500::~MPU6500() {
    deinit();
}

// Initialize the device with the I2C bus handle
esp_err_t MPU6500::init(i2c_master_bus_handle_t bus_handle) {
    if (bus_handle == nullptr) {
        ESP_LOGE(TAG, "Invalid bus handle");
        return ESP_ERR_INVALID_ARG;
    }

    // Configure the device
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = 100000, // 100kHz
    };

    // Add device to the bus
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device to I2C bus: %s", esp_err_to_name(err));
        return err;
    }

    // Check WHO_AM_I (optional)
    uint8_t whoami;
    err = read_whoami(&whoami);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I register");
        return err;
    }
    ESP_LOGI(TAG, "WHO_AM_I: 0x%02X", whoami);
    if (whoami != 0x70) {
        ESP_LOGW(TAG, "Unexpected WHO_AM_I value: 0x%02X (expected 0x70)", whoami);
        // Don't return error - some MPU6500 variants may have different values
    }

    // 1. Wake up device (disable sleep, use best clock)
    err = write_register(PWR_MGMT_1, 0x01);      // Clock: PLL with X-axis gyro reference
    if (err != ESP_OK) return err;
    
    // 2. Configure Digital Low-Pass Filter (DLPF)
    err = write_register(CONFIG, 0x00);          // DLPF_CFG = 0 (Gyro: 250Hz BW)
    if (err != ESP_OK) return err;

    // 3. Configure sensors:
    err = write_register(GYRO_CONFIG, 0x00);     // Gyro: ±250°/s range
    if (err != ESP_OK) return err;
    
    err = write_register(ACCEL_CONFIG, 0x00);    // Accel: ±2g range
    if (err != ESP_OK) return err;
    
    // 4. Enable Accel DLPF (disable A_DLPF_CFG bypass)
    err = write_register(ACCEL_CONFIG2, 0x01);   // Accel Fchoice_b = 0 (enable DLPF) 184Hz
    if (err != ESP_OK) return err;

    // 5. Sample Rate = 1kHz / (1 + SMPLRT_DIV)
    err = write_register(SMPLRT_DIV, 0x04);      // Sample rate = 200Hz (1kHz/(1+4))
    if (err != ESP_OK) return err;
    
    ESP_LOGI(TAG, "MPU6500 initialized successfully");
    return ESP_OK;
}

// Deinitialize the device
esp_err_t MPU6500::deinit() {
    if (dev_handle != nullptr) {
        esp_err_t err = i2c_master_bus_rm_device(dev_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to remove device from I2C bus: %s", esp_err_to_name(err));
        }
        dev_handle = nullptr;
    }
    return ESP_OK;
}

// Register write helper
esp_err_t MPU6500::write_register(uint8_t reg, uint8_t value) {
    if (dev_handle == nullptr) {
        ESP_LOGE(TAG, "Device not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t data[2] = {reg, value};
    esp_err_t err = i2c_master_transmit(dev_handle, data, 2, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg, esp_err_to_name(err));
    }
    return err;
}

// Register read helper
esp_err_t MPU6500::read_register(uint8_t reg, uint8_t *data, size_t len) {
    if (dev_handle == nullptr) {
        ESP_LOGE(TAG, "Device not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg, esp_err_to_name(err));
    }
    return err;
}

// WHO_AM_I check
esp_err_t MPU6500::read_whoami(uint8_t *who_am_i) {
    return read_register(WHO_AM_I, who_am_i, 1);
}

// Data reading
esp_err_t MPU6500::read_data(float* accel_x, float* accel_y, float* accel_z,
                             float* gyro_x, float* gyro_y, float* gyro_z) {
    if (dev_handle == nullptr) {
        ESP_LOGE(TAG, "Device not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t data[14];
    esp_err_t err = read_register(0x3B, data, 14);
    if (err != ESP_OK) {
        return err;
    }

    // Accelerometer data (16-bit)
    int16_t ax = (data[0] << 8) | data[1];
    int16_t ay = (data[2] << 8) | data[3];
    int16_t az = (data[4] << 8) | data[5];
    
    // Gyroscope data (16-bit)
    int16_t gx = (data[8] << 8) | data[9];
    int16_t gy = (data[10] << 8) | data[11];
    int16_t gz = (data[12] << 8) | data[13];
    
    // Convert to physical values
    const float accel_scale = 2.0f / 32768.0f;  // ±2g range
    const float gyro_scale = 250.0f / 32768.0f * (3.1415926535f / 180.0f);  // ±250dps to rad/s
    
    *accel_x = ax * accel_scale;
    *accel_y = ay * accel_scale;
    *accel_z = az * accel_scale;
    
    *gyro_x = gx * gyro_scale;
    *gyro_y = gy * gyro_scale;
    *gyro_z = gz * gyro_scale;
    
    return ESP_OK;
}
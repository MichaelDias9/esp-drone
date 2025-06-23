idf_component_register(
  SRCS "main.c"
  INCLUDE_DIRS "."
  REQUIRES web_socket_client mpu6500 i2c_manager nvs_flash esp_wifi 
)

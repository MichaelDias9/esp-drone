idf_component_register(
  SRCS "main.cpp"
  INCLUDE_DIRS "."
  REQUIRES web_socket_client mpu6500 i2c_manager WifiManager
)

idf_component_register(
  SRCS "main.cpp"
  INCLUDE_DIRS "."
  REQUIRES WifiManager web_socket_client i2c_manager MPU6500
)

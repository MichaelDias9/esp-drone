idf_component_register(
  SRCS "main.cpp"
  INCLUDE_DIRS "."
  REQUIRES  i2c_manager MPU6500 USBComm
)

idf_component_register(
  SRCS "mpu6500.cpp"
  INCLUDE_DIRS "."
  REQUIRES driver i2c_manager sensor_calibration
)
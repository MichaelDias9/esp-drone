idf_component_register(
  SRCS "mpu6500.c"
  INCLUDE_DIRS "."
  REQUIRES driver i2c_manager sensor_calibration
)
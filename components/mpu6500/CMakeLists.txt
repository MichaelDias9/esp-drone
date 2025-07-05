idf_component_register(
  SRCS "Mpu6500.cpp"
  INCLUDE_DIRS "."
  REQUIRES driver i2c_manager 
)
idf_component_register(
  SRCS "WifiManager.cpp"
  INCLUDE_DIRS "."
  REQUIRES nvs_flash esp_wifi 
)
idf_component_register(
  SRCS "web_socket_client.c"
  INCLUDE_DIRS "."
  REQUIRES lwip esp_websocket_client 
)
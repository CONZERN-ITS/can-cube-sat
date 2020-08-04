idf_component_register(SRCS "./Src/sdio.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES router its_config mavlink_comp fatfs esp_serial_slave_link)
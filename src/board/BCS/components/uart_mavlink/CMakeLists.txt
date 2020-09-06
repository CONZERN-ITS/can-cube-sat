idf_component_register(SRCS "./Src/uart_mavlink.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES mavlink_comp router log_collector)
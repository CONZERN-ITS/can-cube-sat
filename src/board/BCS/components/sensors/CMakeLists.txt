idf_component_register(SRCS "./Src/sensors.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES ds18b20 its_config mavlink_comp router)
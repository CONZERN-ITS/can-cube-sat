idf_component_register(SRCS "./Src/sensors.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES ds18b20 mavlink_comp router)
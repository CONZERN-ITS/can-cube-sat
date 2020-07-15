idf_component_register(SRCS "./Src/imi.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES my_i2c its_config mavlink_comp)
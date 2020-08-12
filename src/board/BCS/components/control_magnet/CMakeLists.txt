idf_component_register(SRCS "./Src/control_magnet.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES router its_config mavlink_comp shift_reg)
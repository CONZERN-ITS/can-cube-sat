idf_component_register(SRCS "./Src/control_heat.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES shift_reg
                       PRIV_REQUIRES router its_config mavlink_comp log_collector radio)
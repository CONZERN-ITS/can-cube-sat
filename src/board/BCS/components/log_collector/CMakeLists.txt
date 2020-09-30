idf_component_register(SRCS "./Src/log_collector.c"
                       INCLUDE_DIRS "./Inc"
                       PRIV_REQUIRES mavlink_comp router its_config
                       REQUIRES shift_reg)
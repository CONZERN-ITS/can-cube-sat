idf_component_register(SRCS "./Src/operator.c" "./Src/operator_ip.c"
                       INCLUDE_DIRS "./Inc"
                       PRIV_REQUIRES mavlink_comp router its_config)
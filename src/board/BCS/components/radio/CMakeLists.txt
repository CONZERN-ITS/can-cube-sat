idf_component_register(SRCS "./Src/send.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES router its_config mavlink_comp)
idf_component_register(SRCS "./Src/time_sync.c" "./Src/ark_time_sync.c" "./Src/ntp_server.c"
                       INCLUDE_DIRS "./Inc"
                       REQUIRES router its_config imi)
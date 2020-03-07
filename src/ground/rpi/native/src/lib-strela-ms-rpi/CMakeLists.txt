cmake_minimum_required(VERSION 3.7)

# set the project name
project(strela-ms-rpi-native LANGUAGES C)

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR})

# add the executable
add_library(strela-ms-rpi SHARED
	${SRCDIR}/lis3mdl_STdC/driver/lis3mdl_reg.h
	${SRCDIR}/lis3mdl_STdC/driver/lis3mdl_reg.c
	${SRCDIR}/lsm6ds3_STdC/driver/lsm6ds3_reg.h
	${SRCDIR}/lsm6ds3_STdC/driver/lsm6ds3_reg.c
	${SRCDIR}/strela-ms-rpi.c
)
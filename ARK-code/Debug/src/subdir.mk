################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Time.c \
../src/bme280.c \
../src/bme280_math.c \
../src/ds18b20.c \
../src/ina219.c \
../src/main.c \
../src/my_debug.c \
../src/onewire.c \
../src/sensors.c \
../src/stm32f1xx_hal_msp.c \
../src/stm32f1xx_it.c 

OBJS += \
./src/Time.o \
./src/bme280.o \
./src/bme280_math.o \
./src/ds18b20.o \
./src/ina219.o \
./src/main.o \
./src/my_debug.o \
./src/onewire.o \
./src/sensors.o \
./src/stm32f1xx_hal_msp.o \
./src/stm32f1xx_it.o 

C_DEPS += \
./src/Time.d \
./src/bme280.d \
./src/bme280_math.d \
./src/ds18b20.d \
./src/ina219.d \
./src/main.d \
./src/my_debug.d \
./src/onewire.d \
./src/sensors.d \
./src/stm32f1xx_hal_msp.d \
./src/stm32f1xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F103xB -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1xx" -I"../system/include/cmsis/device" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



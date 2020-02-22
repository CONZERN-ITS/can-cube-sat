################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/Src/tmain.c 

OBJS += \
./Application/Src/tmain.o 

C_DEPS += \
./Application/Src/tmain.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Src/tmain.o: ../Application/Src/tmain.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -DTRACE -c -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Application/Src/tmain.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"


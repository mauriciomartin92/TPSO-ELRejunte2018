################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/miAccesoConfiguracion.c \
../src/miSerializador.c \
../src/misSockets.c 

OBJS += \
./src/miAccesoConfiguracion.o \
./src/miSerializador.o \
./src/misSockets.o 

C_DEPS += \
./src/miAccesoConfiguracion.d \
./src/miSerializador.d \
./src/misSockets.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



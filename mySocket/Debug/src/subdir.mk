################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/accesoConfiguracion.c \
../src/hilo.c \
../src/socket.c 

OBJS += \
./src/accesoConfiguracion.o \
./src/hilo.o \
./src/socket.o 

C_DEPS += \
./src/accesoConfiguracion.d \
./src/hilo.d \
./src/socket.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



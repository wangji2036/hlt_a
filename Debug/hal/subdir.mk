################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hal/_hal.c \
../hal/badc.c \
../hal/bpwm.c \
../hal/ddm.c \
../hal/eadc.c \
../hal/ecap.c \
../hal/epwm.c \
../hal/fmc.c \
../hal/gpio.c \
../hal/i2cm.c \
../hal/i2cs.c \
../hal/isr.c \
../hal/nu6801.c \
../hal/nu6805.c \
../hal/sys.c \
../hal/tcpc.c \
../hal/timer.c \
../hal/uart.c \
../hal/vic.c \
../hal/wdt.c 

OBJS += \
./hal/_hal.o \
./hal/badc.o \
./hal/bpwm.o \
./hal/ddm.o \
./hal/eadc.o \
./hal/ecap.o \
./hal/epwm.o \
./hal/fmc.o \
./hal/gpio.o \
./hal/i2cm.o \
./hal/i2cs.o \
./hal/isr.o \
./hal/nu6801.o \
./hal/nu6805.o \
./hal/sys.o \
./hal/tcpc.o \
./hal/timer.o \
./hal/uart.o \
./hal/vic.o \
./hal/wdt.o 

C_DEPS += \
./hal/_hal.d \
./hal/badc.d \
./hal/bpwm.d \
./hal/ddm.d \
./hal/eadc.d \
./hal/ecap.d \
./hal/epwm.d \
./hal/fmc.d \
./hal/gpio.d \
./hal/i2cm.d \
./hal/i2cs.d \
./hal/isr.d \
./hal/nu6801.d \
./hal/nu6805.d \
./hal/sys.d \
./hal/tcpc.d \
./hal/timer.d \
./hal/uart.d \
./hal/vic.d \
./hal/wdt.d 


# Each subdirectory must supply rules for building sources it contributes
hal/%.o: ../hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CSky Elf C Compiler'
	csky-abiv2-elf-gcc -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -ffunction-sections -fdata-sections -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw -o "$@" "$<" && \
	echo -n '$(@:%.o=%.d)' $(dir $@) > '$(@:%.o=%.d)' && \
	csky-abiv2-elf-gcc -MM -MG -P -w -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -ffunction-sections -fdata-sections -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw   "$<" >> '$(@:%.o=%.d)'
	@echo 'Finished building: $<'
	@echo ' '



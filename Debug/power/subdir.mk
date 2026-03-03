################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../power/bat.c \
../power/buckboost.c 

OBJS += \
./power/bat.o \
./power/buckboost.o 

C_DEPS += \
./power/bat.d \
./power/buckboost.d 


# Each subdirectory must supply rules for building sources it contributes
power/%.o: ../power/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CSky Elf C Compiler'
	csky-abiv2-elf-gcc -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -ffunction-sections -fdata-sections -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw -o "$@" "$<" && \
	echo -n '$(@:%.o=%.d)' $(dir $@) > '$(@:%.o=%.d)' && \
	csky-abiv2-elf-gcc -MM -MG -P -w -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -ffunction-sections -fdata-sections -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw   "$<" >> '$(@:%.o=%.d)'
	@echo 'Finished building: $<'
	@echo ' '



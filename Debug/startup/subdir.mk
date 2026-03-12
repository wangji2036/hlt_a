################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
S_UPPER_SRCS += \
../startup/crt0.S \

OBJS += \
./startup/crt0.o \

S_UPPER_DEPS += \
./startup/crt0.d \


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: CSky Elf Assembler'
	csky-abiv2-elf-gcc -mcpu=ck802 -c -Wa,--gdwarf2 -DDEBUG_SWITCH -Wa,-melrw -o "$@" "$<" && \
	echo -n '$(@:%.o=%.d)' $(dir $@) > '$(@:%.o=%.d)' && \
	csky-abiv2-elf-gcc -MM -MG -P -w -mcpu=ck802 -c -Wa,--gdwarf2 -DDEBUG_SWITCH -Wa,-melrw   "$<" >> '$(@:%.o=%.d)'
	@echo 'Finished building: $<'
	@echo ' '


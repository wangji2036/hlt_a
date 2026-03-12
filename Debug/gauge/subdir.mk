################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../gauge/BMS_FixPoint.c \
../gauge/BMS_FixPoint_data.c \
../gauge/BMS_data.c \
../gauge/SOC.c \
../gauge/SOCPack.c \

OBJS += \
./gauge/BMS_FixPoint.o \
./gauge/BMS_FixPoint_data.o \
./gauge/BMS_data.o \
./gauge/SOC.o \
./gauge/SOCPack.o \

C_DEPS += \
./gauge/BMS_FixPoint.d \
./gauge/BMS_FixPoint_data.d \
./gauge/BMS_data.d \
./gauge/SOC.d \
./gauge/SOCPack.d \


# Each subdirectory must supply rules for building sources it contributes
gauge/%.o: ../gauge/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CSky Elf C Compiler'
	csky-abiv2-elf-gcc -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -c -mistack -ffixed-r8 --std=c99 -Wa,-melrw -ffunction-sections -fdata-sections -o "$@" "$<" && \
	echo -n '$(@:%.o=%.d)' $(dir $@) > '$(@:%.o=%.d)' && \
	csky-abiv2-elf-gcc -MM -MG -P -w -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -c -mistack -ffixed-r8 --std=c99 -Wa,-melrw -ffunction-sections -fdata-sections   "$<" >> '$(@:%.o=%.d)'
	@echo 'Finished building: $<'
	@echo ' '


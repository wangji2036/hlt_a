################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fml/_fml.c \
../fml/adp.c \
../fml/bsp.c \
../fml/dpdm.c \
../fml/fm1210.c \
../fml/fsk.c \
../fml/g_data.c \
../fml/ntc.c \
../fml/nu103x.c \
../fml/qdt.c \
../fml/t91206.c \
../fml/tcpm.c \
../fml/usb_qc.c 

OBJS += \
./fml/_fml.o \
./fml/adp.o \
./fml/bsp.o \
./fml/dpdm.o \
./fml/fm1210.o \
./fml/fsk.o \
./fml/g_data.o \
./fml/ntc.o \
./fml/nu103x.o \
./fml/qdt.o \
./fml/t91206.o \
./fml/tcpm.o \
./fml/usb_qc.o 

C_DEPS += \
./fml/_fml.d \
./fml/adp.d \
./fml/bsp.d \
./fml/dpdm.d \
./fml/fm1210.d \
./fml/fsk.d \
./fml/g_data.d \
./fml/ntc.d \
./fml/nu103x.d \
./fml/qdt.d \
./fml/t91206.d \
./fml/tcpm.d \
./fml/usb_qc.d 


# Each subdirectory must supply rules for building sources it contributes
fml/%.o: ../fml/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CSky Elf C Compiler'
	csky-abiv2-elf-gcc -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -ffunction-sections -fdata-sections -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw -o "$@" "$<" && \
	echo -n '$(@:%.o=%.d)' $(dir $@) > '$(@:%.o=%.d)' && \
	csky-abiv2-elf-gcc -MM -MG -P -w -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -ffunction-sections -fdata-sections -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw   "$<" >> '$(@:%.o=%.d)'
	@echo 'Finished building: $<'
	@echo ' '



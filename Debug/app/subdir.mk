################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/_wpc.c \
../app/app.c \
../app/bat_record.c \
../app/epp.c \
../app/fod.c \
../app/gui.c \
../app/led.c \
../app/main.c \
../app/mpp.c \
../app/pid.c \
../app/port_manager.c \
../app/prot.c \
../app/qfod.c \
../app/sleep.c \
../app/usb_bridge.c \
../app/wpc_5_xfer_1_bpp.c \
../app/wpc_5_xfer_2_epp.c \
../app/wpc_5_xfer_3_mpp.c \
../app/wpc_5_xfer_4_dstrm.c \
../app/wpc_6_test_1_ioc.c \
../app/wpc_6_test_2_iop.c \
../app/wpc_cnfg.c \
../app/wpc_idle.c \
../app/wpc_nego.c \
../app/wpc_ping.c \
../app/wpc_xfer.c 

OBJS += \
./app/_wpc.o \
./app/app.o \
./app/bat_record.o \
./app/epp.o \
./app/fod.o \
./app/gui.o \
./app/led.o \
./app/main.o \
./app/mpp.o \
./app/pid.o \
./app/port_manager.o \
./app/prot.o \
./app/qfod.o \
./app/sleep.o \
./app/usb_bridge.o \
./app/wpc_5_xfer_1_bpp.o \
./app/wpc_5_xfer_2_epp.o \
./app/wpc_5_xfer_3_mpp.o \
./app/wpc_5_xfer_4_dstrm.o \
./app/wpc_6_test_1_ioc.o \
./app/wpc_6_test_2_iop.o \
./app/wpc_cnfg.o \
./app/wpc_idle.o \
./app/wpc_nego.o \
./app/wpc_ping.o \
./app/wpc_xfer.o 

C_DEPS += \
./app/_wpc.d \
./app/app.d \
./app/bat_record.d \
./app/epp.d \
./app/fod.d \
./app/gui.d \
./app/led.d \
./app/main.d \
./app/mpp.d \
./app/pid.d \
./app/port_manager.d \
./app/prot.d \
./app/qfod.d \
./app/sleep.d \
./app/usb_bridge.d \
./app/wpc_5_xfer_1_bpp.d \
./app/wpc_5_xfer_2_epp.d \
./app/wpc_5_xfer_3_mpp.d \
./app/wpc_5_xfer_4_dstrm.d \
./app/wpc_6_test_1_ioc.d \
./app/wpc_6_test_2_iop.d \
./app/wpc_cnfg.d \
./app/wpc_idle.d \
./app/wpc_nego.d \
./app/wpc_ping.d \
./app/wpc_xfer.d 


# Each subdirectory must supply rules for building sources it contributes
app/%.o: ../app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CSky Elf C Compiler'
	csky-abiv2-elf-gcc -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw -o "$@" "$<" && \
	echo -n '$(@:%.o=%.d)' $(dir $@) > '$(@:%.o=%.d)' && \
	csky-abiv2-elf-gcc -MM -MG -P -w -mcpu=ck802 -DDEBUG_SWITCH -Os -g3 -Wall -c  -mistack -ffixed-r8 --std=c99 -Wa,-melrw   "$<" >> '$(@:%.o=%.d)'
	@echo 'Finished building: $<'
	@echo ' '



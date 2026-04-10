################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include app/subdir.mk
-include fml/subdir.mk
-include gauge/subdir.mk
-include hal/subdir.mk
-include lib/subdir.mk
-include osal/subdir.mk
-include power/subdir.mk
-include startup/subdir.mk
-include usbpd/subdir.mk
-include util/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables

# All Target
all: NU17113A1_APROM_IP162_GB_NewPcb.elf NU17113A1_APROM_IP162_GB_NewPcb.ihex NU17113A1_APROM_IP162_GB_NewPcb.bin NU17113A1_APROM_IP162_GB_NewPcb.elf.asm

# Output Target
NU17113A1_APROM_IP162_GB_NewPcb.ihex: NU17113A1_APROM_IP162_GB_NewPcb.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objcopy -O ihex "$<" "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

NU17113A1_APROM_IP162_GB_NewPcb.bin: NU17113A1_APROM_IP162_GB_NewPcb.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objcopy -O binary "$<" "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

NU17113A1_APROM_IP162_GB_NewPcb.elf.asm: NU17113A1_APROM_IP162_GB_NewPcb.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objdump -d "$<" > "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

# Tool invocations
NU17113A1_APROM_IP162_GB_NewPcb.elf: $(OBJS) $(USER_OBJS) ../.claude/worktrees/port0-debug/startup/ckcpu.ld
	@echo 'Building target: $@'
	@echo 'Invoking: CSky Elf Linker'
	csky-abiv2-elf-gcc -mcpu=ck802 -nostartfiles -Wl,--gc-sections -T"../.claude/worktrees/port0-debug/startup/ckcpu.ld" -o "NU17113A1_APROM_IP162_GB_NewPcb.elf" $(OBJS) $(USER_OBJS) $(LIBS) -Wl,-ckmap='$(@:%.elf=%.map)'
	@echo ' '
	@echo 'Size of $@:'
	@csky-abiv2-elf-size "$@"
	@echo ' '
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) NU17113A1_APROM_IP162_GB_NewPcb.ihex NU17113A1_APROM_IP162_GB_NewPcb.bin NU17113A1_APROM_IP162_GB_NewPcb.elf.asm NU17113A1_APROM_IP162_GB_NewPcb.map
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS) NU17113A1_APROM_IP162_GB_NewPcb.elf
	-@echo ' '

.PHONY: all clean dependents


-include ../makefile.targets

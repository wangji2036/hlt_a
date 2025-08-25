################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include util/subdir.mk
-include usbpd/subdir.mk
-include startup/subdir.mk
-include power/subdir.mk
-include osal/subdir.mk
-include hal/subdir.mk
-include gauge/subdir.mk
-include fml/subdir.mk
-include app/subdir.mk
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
all: PowerBankEvk_relsease.elf PowerBankEvk_relsease.ihex PowerBankEvk_relsease.bin PowerBankEvk_relsease.elf.asm

# Output Target
PowerBankEvk_relsease.ihex: PowerBankEvk_relsease.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objcopy -O ihex "$<" "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

PowerBankEvk_relsease.bin: PowerBankEvk_relsease.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objcopy -O binary "$<" "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

PowerBankEvk_relsease.elf.asm: PowerBankEvk_relsease.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objdump -d "$<" > "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

# Tool invocations
PowerBankEvk_relsease.elf: $(OBJS) $(USER_OBJS) C:\Users\senmiao.zhang\Desktop\IP162\IP162_NEW_725_SS\IP162_NEW_725_SS\IP162_NEW_725\PowerBankEvk_relsease\startup\ckcpu.ld
	@echo 'Building target: $@'
	@echo 'Invoking: CSky Elf Linker'
	csky-abiv2-elf-gcc -mcpu=ck802 -nostartfiles -Wl,--gc-sections  -T"C:\Users\senmiao.zhang\Desktop\IP162\IP162_NEW_725_SS\IP162_NEW_725_SS\IP162_NEW_725\PowerBankEvk_relsease\startup\ckcpu.ld" -o "PowerBankEvk_relsease.elf" $(OBJS) $(USER_OBJS) $(LIBS) -Wl,-ckmap='$(@:%.elf=%.map)' 
	@echo ' '
	@echo 'Size of $@:'
	@csky-abiv2-elf-size "$@"
	@echo ' '
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) PowerBankEvk_relsease.ihex PowerBankEvk_relsease.bin PowerBankEvk_relsease.elf.asm PowerBankEvk_relsease.map
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS) PowerBankEvk_relsease.elf
	-@echo ' '

.PHONY: all clean dependents


-include ../makefile.targets

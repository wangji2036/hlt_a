################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include util/subdir.mk
-include startup/subdir.mk
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
all: PowerBankEvk.elf PowerBankEvk.ihex PowerBankEvk.bin PowerBankEvk.elf.asm

# Output Target
PowerBankEvk.ihex: PowerBankEvk.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objcopy -O ihex "$<" "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

PowerBankEvk.bin: PowerBankEvk.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objcopy -O binary "$<" "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

PowerBankEvk.elf.asm: PowerBankEvk.elf
	@echo 'Outputing file $@'
	csky-abiv2-elf-objdump -d "$<" > "$@"
	@echo 'Finished outputing: $@'
	@echo ' '

# Tool invocations
PowerBankEvk.elf: $(OBJS) $(USER_OBJS) D:\Mason\power\ bank\nu17112_powerbank~~~~~new\startup\ckcpu.ld
	@echo 'Building target: $@'
	@echo 'Invoking: CSky Elf Linker'
	csky-abiv2-elf-gcc -mcpu=ck802 -nostartfiles -Wl,--gc-sections  -T"D:\Mason\power bank\nu17112_powerbank~~~~~new\startup\ckcpu.ld" -o "PowerBankEvk.elf" $(OBJS) $(USER_OBJS) $(LIBS) -Wl,-ckmap='$(@:%.elf=%.map)' 
	@echo ' '
	@echo 'Size of $@:'
	@csky-abiv2-elf-size "$@"
	@echo ' '
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) PowerBankEvk.ihex PowerBankEvk.bin PowerBankEvk.elf.asm PowerBankEvk.map
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS) PowerBankEvk.elf
	-@echo ' '

.PHONY: all clean dependents


-include ../makefile.targets

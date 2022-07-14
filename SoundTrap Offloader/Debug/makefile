################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv8/tools/compiler/c5500_4.4.1

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./config.obj" \
"./crc.obj" \
"./crc_asm.obj" \
"./dma.obj" \
"./gpio.obj" \
"./hid.obj" \
"./ioExpander.obj" \
"./irq.obj" \
"./memCardSelect.obj" \
"./mspif.obj" \
"./sar.obj" \
"./sd.obj" \
"./serialEepromV2.obj" \
"./sine.obj" \
"./uart.obj" \
"./src/Commands.obj" \
"./src/MessageProcessor.obj" \
"./src/audioTest.obj" \
"./src/flash.obj" \
"./src/i2cPolled.obj" \
"./src/irCom.obj" \
"./src/main.obj" \
"./src/msp.obj" \
"./src/psense.obj" \
"./src/spi.obj" \
"./src/sysControl.obj" \
"./src/tick.obj" \
"./src/timer.obj" \
"./src/usb.obj" \
"../SUDAR OFFLOADER.cmd" \
$(GEN_CMDS__FLAG) \
-l"C:/Users/Middy/workspace_v8/cslVC5505/Release/cslVC5505.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include src/subdir_vars.mk
-include targetConfigs/subdir_vars.mk
-include subdir_rules.mk
-include src/subdir_rules.mk
-include targetConfigs/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
SoundTrapOffloader.out \

EXE_OUTPUTS__QUOTED += \
"SoundTrapOffloader.out" \

BIN_OUTPUTS += \
SoundTrapOffloader.hex \

BIN_OUTPUTS__QUOTED += \
"SoundTrapOffloader.hex" \


# All Target
all: SoundTrapOffloader.out

# Tool invocations
SoundTrapOffloader.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C5500 Linker'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic -z -m"SoundTrapOffloader.map" --stack_size=0x200 --heap_size=0x400 -i"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/lib" -i"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" -i"C:/ti/bios_5_42_01_09/packages/ti/rtdx/lib/c5500" -i"C:/ti/bios_5_42_01_09/packages/ti/bios/lib" -i"C:/ti/bios_5_42_01_09/packages/ti/rtdx/lib/c5500" -i"C:/ti/bios_5_42_01_09/packages/ti/bios/lib" --reread_libs --warn_sections --xml_link_info="SoundTrapOffloader_linkInfo.xml" --rom_model --sys_stacksize=0x200 -o "SoundTrapOffloader.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

SoundTrapOffloader.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: C5500 Hex Utility'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/hex55"  -o "SoundTrapOffloader.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "config.obj" "crc.obj" "crc_asm.obj" "dma.obj" "gpio.obj" "hid.obj" "ioExpander.obj" "irq.obj" "memCardSelect.obj" "mspif.obj" "sar.obj" "sd.obj" "serialEepromV2.obj" "sine.obj" "uart.obj" "src\Commands.obj" "src\MessageProcessor.obj" "src\audioTest.obj" "src\flash.obj" "src\i2cPolled.obj" "src\irCom.obj" "src\main.obj" "src\msp.obj" "src\psense.obj" "src\spi.obj" "src\sysControl.obj" "src\tick.obj" "src\timer.obj" "src\usb.obj" 
	-$(RM) "crc_asm.d" 
	-$(RM) "config.d" "crc.d" "dma.d" "gpio.d" "hid.d" "ioExpander.d" "irq.d" "memCardSelect.d" "mspif.d" "sar.d" "sd.d" "serialEepromV2.d" "uart.d" "src\Commands.d" "src\MessageProcessor.d" "src\audioTest.d" "src\flash.d" "src\i2cPolled.d" "src\irCom.d" "src\main.d" "src\msp.d" "src\psense.d" "src\spi.d" "src\sysControl.d" "src\tick.d" "src\timer.d" "src\usb.d" 
	-$(RM) "sine.d" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/hex55.exe" -i "SoundTrapOffloader.out" -o "SoundTrapOffloader.bin" -boot -v5505 -b -serial8
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

